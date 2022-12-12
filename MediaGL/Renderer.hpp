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

#pragma region Shaders
const char* PRIMITIVE_VERT_SHADER = R"""(
#version 330 core
layout (location = 0) in vec2 vertex;

uniform mat4 model;
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

const char* SPRITE_VERT_SHADER_OLD = R"""(
#version 330 core
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
}
)""";

const char* SPRITE_VERT_SHADER = R"""(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 modProj;
uniform bool uvflipy = false;

void main()
{
    TexCoords.x = vertex.z;

    if (uvflipy)
         { TexCoords.y = 1.0 - vertex.w; }
    else { TexCoords.y = vertex.w; }
    
    gl_Position = modProj * vec4(vertex.x, vertex.y, 0.0, 1.0);
}
)""";

const char* SPRITE_FRAG_SHADER = R"""(
#version 330 core
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

    // READ ONLY VARS

    Window* window = nullptr;

    View _view;
    glm::mat4 _projection;

    ColorRGBAf _spriteShaderColorState = {1,1,1,1};
    Shader _spriteShader;
    Shader _primShader;
    Shader _textShader;
    VertexArray _spriteVAO{NoCreate};
    VertexBuffer _spriteVBO{NoCreate};

    VertexArray _linesVAO{NoCreate};
    VertexBuffer _linesVBO{NoCreate};
    Rectf _scissor;

    mutable Rectf _cachedVirtualViewport;
    mutable bool _virtViewportDirty = true;

    Renderer(Window& window) { create(window); }
    Renderer() { }

    void create(Window& window)
    {
        this->window = &window;

        // Enable transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Primitives setup
        _primShader.create(PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER);

        // Sprite setup
        _spriteShader.create(SPRITE_VERT_SHADER, SPRITE_FRAG_SHADER);
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

        // Lines setup
        _linesVAO.create();
        _linesVBO.create();

        // Text setup
        _textShader.create(TEXT_VERT_SHADER, TEXT_FRAG_SHADER);

        SDL_AddEventWatch(SDLEventFilterCB, this);
        setView(getDefaultWindowView(window));
        _onWindowResized();
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
        // Old, doesnt work with zoom/rot
        //auto virtRect = getVirtualViewportRect();
        //auto virtSize = Vector2f{ virtRect.width, virtRect.height };
        //Vector2f scale = Vector2(_view.bounds.width, _view.bounds.height) / virtSize;
        //auto mpos = getMouseLocalPos() - Vector2f{ virtRect.x, virtRect.y };
        //auto v = mpos * scale + Vector2f{_view.bounds.x, _view.bounds.y};
        //return v;

        auto rect = getVirtualViewportRect();
        Vector2f mpos = getMouseLocalPos();

        glm::mat4 mat = _view.getMatrix();
        mat = glm::translate(mat, { -mpos.x / _view.zoom.x, -mpos.y / _view.zoom.y, 0.f });
        mat = glm::inverse(mat);
        Vector2f normalized = Vector2f(mat[3].x, mat[3].y) - (Vector2f{ _view.bounds.width, _view.bounds.height } / _view.zoom) / 2;

        return normalized;
    }

    void begin()
    {
        _virtViewportDirty = true;
        resetViewport();
    }

    void clearColor(const ColorRGBAf& color = {0.1f, 0.1f, 0.1f, 1.f})
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void setView(const View& view)
    {
        _view = view;
        _projection = view.getMatrix();

        _spriteShader.bind();
        _spriteShader.setMat4f("projection", _projection);

        _primShader.bind();
        _primShader.setMat4f("projection", _projection);

        _textShader.bind();
        _textShader.setMat4f("projection", _projection);
    }

    void drawTexture(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 }, bool uvflipy = false)
    {
        _spriteShader.bind();

        if (tex.internalFormat == TexInternalFormats::RED)
        { _spriteShader.setBool("grayscale", true); }
        else
        { _spriteShader.setBool("grayscale", false); }

        _spriteShader.setBool("uvflipy", uvflipy);

        // Projection set in setView()

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
    }

    void drawScrollingTexture(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 })
    {

    }

    void drawLine(const Vector2f& start, const Vector2f& end, const ColorRGBAf& color, float width = 1)
    { drawLines(std::vector{ start, end }, color, width); }

    // TODO: batch, it's super slow!!!
    // It's a line loop
    template <typename Vector2fContainer>
    void drawLines(const Vector2fContainer& lines, const ColorRGBAf& color = ColorRGBAf::white(), float width = 1, GLDrawMode mode = GLDrawMode::LINE_STRIP)
    {
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

    static inline View getDefaultWindowView(const Window& win)
    {
        const auto winSize = win.getFramebufferSize();
        return View( 0, 0, winSize.x, winSize.y );
    }

    inline View getDefaultView() { return getDefaultWindowView(*window); }

    bool created() { return window != nullptr; }

    void _onWindowResized()
    {
        resetViewport();
    }

    void resetViewport()
    {
        auto rect = getVirtualViewportRect();

        // OpenGL considers X=0, Y=0 the Lower left Corner of the Screen
        glViewport(rect.x, rect.y, rect.width, rect.height);
    }
};

int SDLEventFilterCB(void* userdata, SDL_Event* event)
{
    Renderer* r = (Renderer*)userdata;
    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
    { r->_onWindowResized(); }
    return 1;
}