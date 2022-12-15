#pragma once

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include "GLHelpers.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Font.hpp"
#include "Shader.hpp"
#include "View.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "SSBO.hpp"


#define STRING_MACRO_NAME(X) #X
#define STRING_MACRO_VALUE(X) STRING_MACRO_NAME(X)

#pragma region Shaders
const char* PRIMITIVE_VERT_SHADER = R"""(
#version 330 core
layout (location = 0) in vec2 vertex;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
}
)""";

const char* PRIMITIVE_FRAG_SHADER = R"""(
#version 330 core
out vec4 FragColor;
uniform vec4 color;

void main()
{
   FragColor = color;
}
)""";

#pragma endregion

template <typename T>
size_t getVecSizeInBytes(const std::vector<T>& vec)
{
    return vec.size() * sizeof(T);
}

int SDLEventFilterCB(void* userdata, SDL_Event* event);

enum class GLDrawMode
{
    POINTS                   = GL_POINTS,
    LINE_STRIP               = GL_LINE_STRIP,
    LINE_LOOP                = GL_LINE_LOOP,
    LINES                    = GL_LINES,
    LINE_STRIP_ADJACENCY     = GL_LINE_STRIP_ADJACENCY,
    LINES_ADJACENCY          = GL_LINES_ADJACENCY,
    TRIANGLE_STRIP           = GL_TRIANGLE_STRIP,
    TRIANGLE_FAN             = GL_TRIANGLE_FAN,
    TRIANGLES                = GL_TRIANGLES,
    TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
    TRIANGLES_ADJACENCY      = GL_TRIANGLES_ADJACENCY,
    PATCHES                  = GL_PATCHES
};

struct Renderer
{
    // Config

    // If true, renderer will use black bars to maintain views aspect ratio.
    // Also adjusts functions for getting mouse position.
    bool maintainAspect = false;
    bool frustumCullingEnabled = true;

    // Read-only

    Window* window = nullptr;
    size_t lastFrameDrawCalls = 0;

    // Private

    bool begun = false;
    View _view;
    glm::mat4 _projection;
    Frustum _frustum;

    Shader _primShader;
    Shader _textShader;

    VertexArray _linesVAO{NoCreate};
    VertexBuffer _linesVBO{NoCreate};

    mutable Rectf _cachedVirtualViewport;
    mutable bool _virtViewportDirty = true;

    size_t _debugDrawCalls = 0;

    Renderer(Window& window) { create(window); }
    Renderer() { }

    #pragma region Misc

    void create(Window& window)
    {
        this->window = &window;

        // Enable transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Primitives setup
        _primShader.create(PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER);

        textureInit();

        // Lines setup
        _linesVAO.create();
        _linesVBO.create();

        // Text setup
        _textShader.create(TEXT_VERT_SHADER, TEXT_FRAG_SHADER);

        SDL_AddEventWatch(SDLEventFilterCB, this);
        setView(getDefaultWindowView(window));
        _onWindowResized();
    }

    bool created() { return window != nullptr; }

    void begin()
    {
        ASSERTMSG(begun == false, "Forgot to call render()");
        begun = true;

        _debugDrawCalls = 0;
        _virtViewportDirty = true;
        resetViewport();
    }

    void render()
    {
        ASSERTMSG(begun == true, "Forgot to call begin()");
        begun = false;
        flushTextureBatch();

        lastFrameDrawCalls = _debugDrawCalls;
        _debugDrawCalls = 0;
    }

    Rectf getVirtualViewportRect() const
    {
        Vector2f winSize = Vector2f(window->getFramebufferSize());

        if (!_virtViewportDirty) { return _cachedVirtualViewport; }
        Rectf vp;

        if (maintainAspect)
        {
            float targetAspectRatio = _view.bounds.width / _view.bounds.height;
            float width = winSize.x ;
            float height = width / targetAspectRatio + 0.5f;

            if (height > winSize.y )
            {
                height = winSize.y;
                width = height * targetAspectRatio + 0.5f;
            }

            float vp_x = (winSize.x / 2) - (width  / 2);
            float vp_y = (winSize.y / 2) - (height / 2);

            vp = Rectf( vp_x, winSize.y - (vp_y + height), width, height );
        }
        else
        {
            auto fb = window->getFramebufferSize();
            vp = Rectf(0, 0, fb.x, fb.y);
        }

        _cachedVirtualViewport = vp;
        _virtViewportDirty = false;
        return vp;
    }

    Vector2f getMouseLocalPos()
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return Vector2f(x, y);
    }

    Vector2f getMouseWorldPos()
    {
        auto rect = getVirtualViewportRect();
        Vector2f mpos = getMouseLocalPos();

        glm::mat4 mat = _view.getMatrix();
        mat = glm::translate(mat, { -mpos.x / _view.zoom.x, -mpos.y / _view.zoom.y, 0.f });
        mat = glm::inverse(mat);
        Vector2f normalized = Vector2f(mat[3].x, mat[3].y) - (Vector2f{ _view.bounds.width, _view.bounds.height } / _view.zoom) / 2;

        return normalized;
    }

    void clearColor(const ColorRGBAf& color = {0.1f, 0.1f, 0.1f, 1.f})
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void setView(const View& view)
    {
        if (view == _view) { return; }

        _view = view;
        _projection = view.getMatrix();
        _frustum = Frustum(_projection);

        _spriteShader.bind();
        _spriteShader.setMat4f("projection", _projection);
        _fastSpriteShader.bind();
        _fastSpriteShader.setMat4f("projection", _projection);
        _primShader.bind();
        _primShader.setMat4f("projection", _projection);
        _textShader.bind();
        _textShader.setMat4f("projection", _projection);
    }

    View getView() const { return _view; }

    static inline View getDefaultWindowView(const Window& win)
    {
        const auto winSize = win.getFramebufferSize();
        return View( 0, 0, winSize.x, winSize.y );
    }

    inline View getDefaultView() { return getDefaultWindowView(*window); }

    void resetViewport()
    {
        auto rect = getVirtualViewportRect();

        // OpenGL considers X=0, Y=0 the Lower left Corner of the Screen
        glViewport(rect.x, rect.y, rect.width, rect.height);
    }

    void _onWindowResized()
    {
        resetViewport();
    }

    #pragma endregion

    #pragma region Texture

    ColorRGBAf   _spriteShaderColorState = {1,1,1,1};
    Shader       _spriteShader;
    VertexArray  _spriteVAO{NoCreate};
    VertexBuffer _spriteVBO{NoCreate};

    // Batched textures

    #ifndef MAX_SPRITE_BATCH_SIZE
    #define MAX_SPRITE_BATCH_SIZE 1024 * 8
    #endif

    struct
    {
        // Index per instance arrays like this : arr[gl_VertexID / vertCount]
        // Index per vertex array like this    : arr[gl_VertexID % vertCount]

        // PER INSTANCE
        std::array<glm::mat4, MAX_SPRITE_BATCH_SIZE> model;
        static_assert( sizeof(glm::mat4) * MAX_SPRITE_BATCH_SIZE == sizeof(std::array<glm::mat4, MAX_SPRITE_BATCH_SIZE>) );

        // PER VERTEX
        std::array<glm::vec2, 6 * MAX_SPRITE_BATCH_SIZE> texCoords;
        static_assert( sizeof(glm::vec2) * 6 * MAX_SPRITE_BATCH_SIZE == sizeof(std::array<glm::vec2, 6 * MAX_SPRITE_BATCH_SIZE>) );

        // PER INSTANCE
        std::array<glm::vec4, MAX_SPRITE_BATCH_SIZE> color;
        static_assert( sizeof(glm::vec4) * MAX_SPRITE_BATCH_SIZE == sizeof(std::array<glm::vec4, MAX_SPRITE_BATCH_SIZE>) );

    } _texBatchData;
    
    int _currentSpriteBatchIndex = 0;
    Shader _fastSpriteShader;
    SSBO _texBatchDataSSBO;
    Texture* _currentTexture = nullptr;

    void textureInit()
    {
        #pragma region slow sprites
        // Sprite setup
        _spriteShader.create(
        // VERT
  R"""( #version 460 core
        layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

        out vec2 TexCoords;

        uniform mat4 model;
        uniform mat4 projection;
        uniform bool uvflipy = false;

        void main()
        {
            TexCoords.x = vertex.z;

            if (uvflipy)
                 { TexCoords.y = 1.0 - vertex.w; }
            else { TexCoords.y = vertex.w; }
    
            gl_Position = projection * model * vec4(vertex.x, vertex.y, 0.0, 1.0);
        } )""", 
        // FRAG
        R"""(
        #version 460 core
        in vec2 TexCoords;
        out vec4 color;

        uniform sampler2D image;
        uniform vec4 spriteColor;
        uniform bool grayscale;

        void main()
        {
            if (grayscale == false)
            { color = spriteColor * texture(image, TexCoords); }
            else
            {
                color = vec4(spriteColor) * vec4(1.0, 1.0, 1.0, texture(image, TexCoords).r);
            }
        } )""");
        _spriteShader.bind();
        _spriteShader.setInt("image", 0);
        _spriteShader.setVec4f("spriteColor", 1, 1, 1, 1);
        _spriteShaderColorState = { 1, 1, 1, 1 };
        _spriteShader.unbind();

        GL_CHECK(glActiveTexture(GL_TEXTURE0));

        float vertices[] = { 
            // pos      // tex
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f, 

            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        };

        _spriteVAO.create();
        _spriteVBO.create();

        _spriteVBO.bind();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        _spriteVAO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        _spriteVBO.unbind();
        _spriteVAO.unbind();

        #pragma endregion

        // Fast sprite setup
        #pragma region Shader Source
        _fastSpriteShader.create(
            // VERTEX
            R"(
            #version 460 core

            vec2 pos[6] = vec2[6](vec2(0.0f, 1.0f),
                                  vec2(1.0f, 0.0f),
                                  vec2(0.0f, 0.0f),
                                  vec2(0.0f, 1.0f),
                                  vec2(1.0f, 1.0f),
                                  vec2(1.0f, 0.0f));

            layout(std430, binding = 0) buffer data
            {
                mat4 model    [)"     STRING_MACRO_VALUE(MAX_SPRITE_BATCH_SIZE) R"(];
                vec2 texCoords[6 * )" STRING_MACRO_VALUE(MAX_SPRITE_BATCH_SIZE) R"(];
                vec4 color    [)"     STRING_MACRO_VALUE(MAX_SPRITE_BATCH_SIZE) R"(];
            };

            out vec2 fragTexCoords;
            out vec4 fragColor;

            uniform mat4 projection;

            void main()
            {
                // gl_VertexID % 6
                fragTexCoords.x = texCoords[gl_VertexID].x;
                fragTexCoords.y = texCoords[gl_VertexID].y;
                fragColor = color[gl_VertexID / 6];
    
                gl_Position = projection * model[gl_VertexID / 6] * vec4(pos[gl_VertexID % 6], 0.0, 1.0);
            }
            )",
            // FRAG
            R"(
            #version 460 core
            in vec2 fragTexCoords;
            in vec4 fragColor;
            out vec4 color;

            uniform sampler2D image;

            void main()
            {
                color = fragColor * texture(image, fragTexCoords);
                //color = vec4(1, 0, 1, 1);
            }
            )"
        );
        #pragma endregion

        _fastSpriteShader.bind();
        _fastSpriteShader.setInt("image", 0);
        _fastSpriteShader.unbind();

        _texBatchDataSSBO.create();
        _texBatchDataSSBO.bind();
    }

    // Prefer drawTextureFast
    void drawTexture(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 }, bool uvflipy = false)
    {
        if (frustumCullingEnabled && !_frustum.IsBoxVisible({ rect.x, rect.y, 0 }, {rect.x + rect.width, rect.y + rect.height, 0})) { return; }
        flushTextureBatch();

        _spriteShader.bind();

        //if (tex.internalFormat == TexInternalFormats::RED)
        //{ _spriteShader.setBool("grayscale", true); }
        //else
        //{ _spriteShader.setBool("grayscale", false); }
        //
        //_spriteShader.setBool("uvflipy", uvflipy);

        glm::mat4 model = glm::mat4(1.0f);
        
        model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));

        if (rot != 0)
        {
            model = glm::translate(model, glm::vec3(0.5f * rect.width, 0.5f * rect.height, 0.0f));
            model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-0.5f * rect.width, -0.5f * rect.height, 0.0f));
        }

        model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.f));
        

        _spriteShader.setMat4f("model", model); // Slow

        if (_spriteShaderColorState != color)
        {
            _spriteShader.setVec4f("spriteColor", color.r, color.g, color.b, color.a);
            _spriteShaderColorState = color;
        }

        tex.bind();
        _spriteVAO.bind();
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));


        ++_debugDrawCalls;
    }

    void drawTextureFast(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 })
    {
        if (frustumCullingEnabled && !_frustum.IsBoxVisible({ rect.x, rect.y, 0 }, {rect.x + rect.width, rect.y + rect.height, 0})) { return; }
        if (&tex != _currentTexture) { flushTextureBatch(); }

        _texBatchData.model[_currentSpriteBatchIndex] = glm::mat4(1.0f);

        _texBatchData.model[_currentSpriteBatchIndex] = glm::translate(_texBatchData.model[_currentSpriteBatchIndex], glm::vec3(rect.x, rect.y, 0.0f));

        if (rot != 0)
        {
            _texBatchData.model[_currentSpriteBatchIndex] =
                glm::translate(_texBatchData.model[_currentSpriteBatchIndex], glm::vec3(0.5f * rect.width, 0.5f * rect.height, 0.0f));
            _texBatchData.model[_currentSpriteBatchIndex] =
                glm::rotate(   _texBatchData.model[_currentSpriteBatchIndex], glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
            _texBatchData.model[_currentSpriteBatchIndex] =
                glm::translate(_texBatchData.model[_currentSpriteBatchIndex], glm::vec3(-0.5f * rect.width, -0.5f * rect.height, 0.0f));
        }

        _texBatchData.model[_currentSpriteBatchIndex] = glm::scale(_texBatchData.model[_currentSpriteBatchIndex], glm::vec3(rect.width, rect.height, 1.f));
        

        _texBatchData.texCoords[0 + _currentSpriteBatchIndex * 6] = { 0.0f, 1.0f };
        _texBatchData.texCoords[1 + _currentSpriteBatchIndex * 6] = { 1.0f, 0.0f };
        _texBatchData.texCoords[2 + _currentSpriteBatchIndex * 6] = { 0.0f, 0.0f };
        _texBatchData.texCoords[3 + _currentSpriteBatchIndex * 6] = { 0.0f, 1.0f };
        _texBatchData.texCoords[4 + _currentSpriteBatchIndex * 6] = { 1.0f, 1.0f };
        _texBatchData.texCoords[5 + _currentSpriteBatchIndex * 6] = { 1.0f, 0.0f };


        _texBatchData.color[_currentSpriteBatchIndex] = { color.r, color.g, color.b, color.a };

        ++_currentSpriteBatchIndex;
        _currentTexture = &tex;

        if (_currentSpriteBatchIndex == MAX_SPRITE_BATCH_SIZE) { flushTextureBatch(); }
    }

    // Call this to draw the batched textures early
    void flushTextureBatch()
    {
        if (_currentTexture == nullptr || _currentSpriteBatchIndex == 0) { return; }

        _texBatchDataSSBO.bind();
        _texBatchDataSSBO.bufferData(_texBatchData);
        _texBatchDataSSBO.setBufferBase(0);

        _fastSpriteShader.bind();
        _currentTexture->bind();
        VertexArray::unbind();
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6 * _currentSpriteBatchIndex));

        _texBatchDataSSBO.unbind();

        ++_debugDrawCalls;
        _currentSpriteBatchIndex = 0;
    }

    #pragma endregion

    #pragma region Primitives

    void drawLine(const Vector2f& start, const Vector2f& end, const ColorRGBAf& color, float width = 1)
    { drawLines(std::vector{ start, end }, color, width); }

    // TODO: batch, it's super slow!!!
    // It's a line loop
    template <typename Vector2fContainer>
    void drawLines(const Vector2fContainer& lines, const ColorRGBAf& color = ColorRGBAf::white(),
                   float width = 1, GLDrawMode mode = GLDrawMode::LINE_STRIP)
    {
        flushTextureBatch();

        _primShader.bind();
        // Projection set in setView()
        _primShader.setVec4f("color", color.r, color.g, color.b, color.a);

        _linesVBO.bind();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vector2f), lines.data(), GL_DYNAMIC_DRAW));

        _linesVAO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0));

        glLineWidth(width);
        GL_CHECK(glDrawArrays((int)mode, 0, lines.size()));

        ++_debugDrawCalls;
    }

    void drawRect(const Rectf& rect, const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false)
    { drawRect(rect.x, rect.y, rect.width, rect.height, color, filled); }

    void drawRect(float x, float y, float w, float h, const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false)
    {
        std::vector<Vector2f> verts(4);
        verts[0] = Vector2f(x,     y);
        verts[1] = Vector2f(x + w, y);
        verts[2] = Vector2f(x + w, y + h);
        verts[3] = Vector2f(x,     y + h);

        drawLines(verts, color, 1, filled ? GLDrawMode::TRIANGLE_FAN : GLDrawMode::LINE_LOOP);
    }

    void drawGrid(const Vector2f& start, const Vector2i& gridCount, Vector2f gridSize, const ColorRGBAf& color)
    {
        const float targetX = gridCount.x * gridSize.x + start.x;
        const float targetY = gridCount.y * gridSize.y + start.y;

        for (int x = 0; x <= gridCount.x; x++)
        {
            const float startx = x + start.x;
            drawLine(
                Vector2f{ startx * gridSize.x, 0 },
                Vector2f{ startx * gridSize.x, targetY },
                color
            );
        }

        for (int y = 0; y <= gridCount.y; y++)
        {
            const float starty = y + start.y;
            drawLine(
                Vector2f{ 0, starty * gridSize.y },
                Vector2f{ targetX, starty * gridSize.y },
                color
            );
        }
    }

    void drawCircle(const Vector2f& pos, const float rad,
                    const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false, const size_t segmentCount = 16)
    {
        const float theta = 3.1415926 * 2 / float(segmentCount);
        const float tangetial_factor = tanf(theta);
        const float radial_factor = cosf(theta);

        float x = rad;
        float y = 0;

        std::vector<Vector2f> points;
        points.reserve(segmentCount);

        for (int i = 0; i < segmentCount; i++)
        {
            points.push_back(Vector2f{ x + pos.x, y + pos.y });

            float tx = -y;
            float ty = x;

            //add the tangential vector 

            x += tx * tangetial_factor;
            y += ty * tangetial_factor;

            //correct using the radial factor 

            x *= radial_factor;
            y *= radial_factor;
        }

        drawLines(points, color, 1.f, filled ? GLDrawMode::TRIANGLE_FAN : GLDrawMode::LINE_LOOP);
    }

    // Pos will be the bottom left of the text
    // Don't use
    void drawText(const std::string& text, Font& font, const Vector2f& pos,
                  const ColorRGBAf& color = ColorRGBAf::white(), float scale = 1.f)
    {
        Vector2f currentPos = pos;
        for (auto& strchar : text)
        {
            // TODO: add default unknown character, and use that instead
            if (!font.characters.contains(strchar))
            { std::cout << "Font does not contain character \"" << strchar << "\"" << std::endl; continue; }

            Font::Character& ch = font.characters.at(strchar);
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;
            float xpos = (currentPos.x + ch.bearing.x);
            float ypos = (currentPos.y - h + (h - ch.bearing.y * scale));

            drawTexture(ch.texture, { xpos, ypos, w, h }, 0.f, color);

            currentPos.x += (ch.advance >> 6) * scale;
        }
    }

    #pragma endregion
};

int SDLEventFilterCB(void* userdata, SDL_Event* event)
{
    Renderer* r = (Renderer*)userdata;
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    { r->_onWindowResized(); }
    return 1;
}