#pragma once

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include "GLHelpers.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"
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

const char* SPRITE_VERT_SHADER = R"""(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
)""";

const char* SPRITE_FRAG_SHADER = R"""(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec4 spriteColor;

void main()
{    
    color = spriteColor * texture(image, TexCoords);
}  
)""";
#pragma endregion

template <typename T>
size_t getVecSizeInBytes(const std::vector<T>& vec)
{
    return vec.size() * sizeof(T);
}

struct Renderer
{
    Window* window = nullptr;
    
    View _view; // Read only
    glm::mat4 _projection;

    ColorRGBAf _spriteShaderColorState = {1,1,1,1};
    Shader _spriteShader;
    Shader _primShader;
    VertexArray _spriteVAO{NoCreate};

    VertexArray _linesVAO{NoCreate};
    VertexBuffer _linesVBO{NoCreate};

    Renderer(Window& window) { create(window); }
    Renderer() { }

    void create(Window& window)
    {
        this->window = &window;

        // Enable transparency
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

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

        GLuint VBO;
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
        GL_CHECK(glGenBuffers(1, &VBO));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        _spriteVAO.bind();
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));  
        _spriteVAO.unbind();

        // Lines setup
        _linesVAO.create();
        _linesVBO.create();

        setView(getDefaultWindowView(window));
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
    }

    void drawTexture(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 })
    {
        _spriteShader.bind();

        // Projection set in setView()

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));

        if (rot != 0)
        {
            model = glm::translate(model, glm::vec3(0.5f * rect.width, 0.5f * rect.height, 0.0f));
            model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(-0.5f * rect.width, -0.5f * rect.height, 0.0f));
        }

        model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f)); 

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

    void drawLine(const Vector2f& start, const Vector2f& end, const ColorRGBAf& color, float width = 1)
    { drawLines({ start, end }, color, width, false); }

    // TODO: batch, it's super slow!!!
    // It's a line loop
    void drawLines(const std::vector<Vector2f>& lines, const ColorRGBAf& color, float width = 1, bool loop = false)
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
        GL_CHECK(glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, lines.size()));
    }

    void drawRect(const Rectf& rect, const ColorRGBAf& color = ColorRGBAf::white())
    { drawRect(rect.x, rect.y, rect.width, rect.height, color); }

    void drawRect(float x, float y, float w, float h, const ColorRGBAf& color = ColorRGBAf::white())
    {
        std::vector<Vector2f> verts(4);
        verts[0] = Vector2f(x,     y);
        verts[1] = Vector2f(x + w, y);
        verts[2] = Vector2f(x + w, y + h);
        verts[3] = Vector2f(x,     y + h);

        drawLines(verts, color, 1, true);
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

    static inline View getDefaultWindowView(const Window& win)
    {
        const auto winSize = win.getSize();
        return View( 0, 0, winSize.x, winSize.y );
    }

    bool created() { return window != nullptr; }
};