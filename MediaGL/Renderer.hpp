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
#include "../Macros.hpp"

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

    // Read / Write
    // If true, renderer will use black bars to maintain views aspect ratio.
    // Also adjusts getMouseWorldPos() to return accurate coords.
    bool maintainAspect = false;       

    // Read / Write
    // Culls things drawn outside the view frustum.
    // It's super fast!! Probably no reason to turn it off.
    bool frustumCullingEnabled = true; 

    // Read-only

    Window* window = nullptr;      // Read only
    SDL_GLContext glContext;       // Read only
    size_t lastFrameDrawCalls = 0; // Read only

    // Private

    bool      begun = false; // Read only
    View      view;         // Read only
    glm::mat4 projection;   // Read only
    Frustum   frustum;      // Read only

    Shader primShader; // Read only
    Shader textShader; // Read only

    VertexArray  linesVAO{NoCreate}; // Read only
    VertexBuffer linesVBO{NoCreate}; // Read only

    mutable Rectf cachedVirtualViewport;   // Read only
    mutable bool virtViewportDirty = true; // Read only

    size_t debugDrawCalls = 0; // Private

    Renderer() = default;
    ~Renderer() { reset(); }

    #pragma region Misc

    void create(Window& window)
    {
        this->window = &window;

        ASSERTMSG(WindowFlags::OpenGL & window.getFlags(),
            "Your window must be created with the WindowFlags::OpenGL flag!");

        glContext = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, glContext);
        SDL_GL_SetSwapInterval(0);

        if (gl3wInit())
        { fprintf(stderr, "failed to initialize OpenGL\n"); abort(); }
        if (!gl3wIsSupported(3, 3))
        { fprintf(stderr, "OpenGL 3.3 not supported\n"); abort(); }

        printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback( defaultGLCallback, 0 );

        // Enable transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Primitives setup
        primShader.create(PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER);

        textureInit();

        // Lines setup
        linesVAO.create();
        linesVBO.create();

        // Text setup
        textShader.create(TEXT_VERT_SHADER, TEXT_FRAG_SHADER);

        SDL_AddEventWatch(SDLEventFilterCB, this);
        setView(getDefaultWindowView(window));
        _onWindowResized();
    }

    bool created() { return window != nullptr; }

    void reset()
    {
        if (!created()) return;
        SDL_GL_DeleteContext(glContext);
    }

    void begin()
    {
        ASSERTMSG(begun == false, "Forgot to call render()");
        begun = true;

        debugDrawCalls = 0;
        virtViewportDirty = true;
        resetViewport();
    }

    void render()
    {
        ASSERTMSG(begun == true, "Forgot to call begin()");
        begun = false;
        flushTextureBatch();

        lastFrameDrawCalls = debugDrawCalls;
        debugDrawCalls = 0;
    }

    Rectf getVirtualViewportRect() const
    {
        Vector2f winSize = Vector2f(window->getFramebufferSize());

        if (!virtViewportDirty) { return cachedVirtualViewport; }
        Rectf vp;

        if (maintainAspect)
        {
            float targetAspectRatio = view.bounds.width / view.bounds.height;
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

        cachedVirtualViewport = vp;
        virtViewportDirty = false;
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

        glm::mat4 mat = view.getMatrix();
        mat = glm::translate(mat, { -mpos.x / view.zoom.x, -mpos.y / view.zoom.y, 0.f });
        mat = glm::inverse(mat);
        Vector2f normalized = Vector2f(mat[3].x, mat[3].y) - (Vector2f{ view.bounds.width, view.bounds.height } / view.zoom) / 2;

        return normalized;
    }

    void clearColor(const ColorRGBAf& color = {0.1f, 0.1f, 0.1f, 1.f})
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void setView(const View& newView)
    {
        if (view == newView) { return; }

        view = newView;
        projection = view.getMatrix();
        frustum = Frustum(projection);

        spriteShader.bind();
        spriteShader.setMat4f("projection", projection);
        fastSpriteShader.bind();
        fastSpriteShader.setMat4f("projection", projection);
        primShader.bind();
        primShader.setMat4f("projection", projection);
        textShader.bind();
        textShader.setMat4f("projection", projection);
    }

    View getView() const { return view; }

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

    ColorRGBAf   spriteShaderColorState = {1,1,1,1};
    Shader       spriteShader;
    VertexArray  spriteVAO{NoCreate};
    VertexBuffer spriteVBO{NoCreate};

    // Batched textures

    #ifndef MAX_SPRITE_BATCH_SIZE
    #define MAX_SPRITE_BATCH_SIZE 1024 * 8
    #endif

    struct FastSpriteInstance
    {
        glm::mat4 model;
        std::array<glm::vec2, 6> texCoords;
        glm::vec4 color;
    };

    std::array<FastSpriteInstance, MAX_SPRITE_BATCH_SIZE> spriteBatch;
    int currentSpriteBatchIndex = 0;
    Shader fastSpriteShader;
    SSBO texBatchDataSSBO;
    Texture* currentTexture = nullptr;

    void textureInit()
    {
        #pragma region slow sprites
        // Sprite setup
        spriteShader.create(
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
        spriteShader.bind();
        spriteShader.setInt("image", 0);
        spriteShader.setVec4f("spriteColor", 1, 1, 1, 1);
        spriteShaderColorState = { 1, 1, 1, 1 };
        spriteShader.unbind();

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

        spriteVAO.create();
        spriteVBO.create();

        spriteVBO.bind();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        spriteVAO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        spriteVBO.unbind();
        spriteVAO.unbind();

        #pragma endregion

        // Fast sprite setup
        #pragma region Shader Source
        fastSpriteShader.create(
            // VERTEX
            R"(
            #version 460 core

            vec2 pos[6] = vec2[6](vec2(0.0f, 1.0f),
                                  vec2(1.0f, 0.0f),
                                  vec2(0.0f, 0.0f),
                                  vec2(0.0f, 1.0f),
                                  vec2(1.0f, 1.0f),
                                  vec2(1.0f, 0.0f));

            struct Sprite
            {
                mat4 model;
                vec2 texCoords[6];
                vec4 color;
            };

            layout(std430, binding = 0) buffer data
            {
                Sprite sprites    [)" STRING_MACRO_VALUE(MAX_SPRITE_BATCH_SIZE) R"(];
            };

            out vec2 fragTexCoords;
            out vec4 fragColor;

            
            uniform mat4 projection;

            void main()
            {
                Sprite sprite = sprites[gl_InstanceID];
                int vert = gl_VertexID;

                fragTexCoords = sprite.texCoords[vert];

                fragColor = sprite.color;
    
                gl_Position = projection * sprite.model * vec4(pos[vert], 0.0, 1.0);
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

        spriteBatch.fill(FastSpriteInstance());

        fastSpriteShader.bind();
        fastSpriteShader.setInt("image", 0);
        fastSpriteShader.unbind();

        texBatchDataSSBO.create();
        texBatchDataSSBO.bind();
    }

    // Prefer drawTextureFast
    void drawTexture(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 }, bool uvflipy = false)
    {
        if (frustumCullingEnabled && !frustum.IsBoxVisible({ rect.x, rect.y, 0 }, {rect.x + rect.width, rect.y + rect.height, 0})) { return; }
        flushTextureBatch();
        
        spriteShader.bind();

        //if (tex.internalFormat == TexInternalFormats::RED)
        //{ spriteShader.setBool("grayscale", true); }
        //else
        //{ spriteShader.setBool("grayscale", false); }
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
        

        spriteShader.setMat4f("model", model); // Slow

        if (spriteShaderColorState != color)
        {
            spriteShader.setVec4f("spriteColor", color.r, color.g, color.b, color.a);
            spriteShaderColorState = color;
        }

        tex.bind();
        spriteVAO.bind();
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));


        ++debugDrawCalls;
    }

    void drawTextureFast(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 })
    {
        if (frustumCullingEnabled && !frustum.IsBoxVisible({ rect.x, rect.y, 0 }, {rect.x + rect.width, rect.y + rect.height, 0})) { return; }
        if (&tex != currentTexture) { flushTextureBatch(); }

        FastSpriteInstance& inst = spriteBatch[currentSpriteBatchIndex];

        inst.model = glm::mat4(1.0f);

        inst.model = glm::translate(inst.model, glm::vec3(rect.x, rect.y, 0.0f));

        if (rot != 0)
        {
            inst.model = glm::translate(inst.model, glm::vec3(0.5f * rect.width, 0.5f * rect.height, 0.0f));
            inst.model = glm::rotate   (inst.model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
            inst.model = glm::translate(inst.model, glm::vec3(-0.5f * rect.width, -0.5f * rect.height, 0.0f));
        }

        inst.model = glm::scale(inst.model, glm::vec3(rect.width, rect.height, 1.f));
        

        inst.texCoords[0] = { 0.0f, 1.0f };
        inst.texCoords[1] = { 1.0f, 0.0f };
        inst.texCoords[2] = { 0.0f, 0.0f };
        inst.texCoords[3] = { 0.0f, 1.0f };
        inst.texCoords[4] = { 1.0f, 1.0f };
        inst.texCoords[5] = { 1.0f, 0.0f };


        inst.color = { color.r, color.g, color.b, color.a };

        ++currentSpriteBatchIndex;
        currentTexture = &tex;

        if (currentSpriteBatchIndex == MAX_SPRITE_BATCH_SIZE) { flushTextureBatch(); }
    }

    // Call this to draw the batched textures early
    void flushTextureBatch()
    {
        if (currentTexture == nullptr || currentSpriteBatchIndex == 0) { return; }

        texBatchDataSSBO.bind();
        texBatchDataSSBO.bufferData(spriteBatch, currentSpriteBatchIndex, GL_DYNAMIC_DRAW);
        texBatchDataSSBO.setBufferBase(0);

        fastSpriteShader.bind();
        currentTexture->bind();
        VertexArray::unbind();
        GL_CHECK(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, currentSpriteBatchIndex));

        texBatchDataSSBO.unbind();

        ++debugDrawCalls;
        currentSpriteBatchIndex = 0;
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

        primShader.bind();
        // Projection set in setView()
        primShader.setVec4f("color", color.r, color.g, color.b, color.a);

        linesVBO.bind();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(Vector2f), lines.data(), GL_DYNAMIC_DRAW));

        linesVAO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0));

        glLineWidth(width);
        GL_CHECK(glDrawArrays((int)mode, 0, lines.size()));

        ++debugDrawCalls;
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