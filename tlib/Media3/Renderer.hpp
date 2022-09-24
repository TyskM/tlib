#pragma once

#include <glm/mat4x4.hpp>
#include <glm/ext.hpp>
#include "GLHelpers.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "RenderTarget.hpp"
#include "View.hpp"

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
protected:
    View view;

public:
    Window* window = nullptr;
    RenderTarget* target = nullptr;

    glm::mat4 projection;

    Shader spriteShader;
    Shader primShader;
    GLuint spriteVAO;

    Renderer(Window& window) { create(window); }
    Renderer() { }

    ~Renderer()
    {
        if (created())
        {
            glDeleteBuffers(1, &spriteVAO);
        }
    }

    void create(Window& window)
    {
        this->window = &window;
        setView(getDefaultWindowView(window));

        // Enable transparency
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // Primitives setup
        primShader.create(PRIMITIVE_VERT_SHADER, PRIMITIVE_FRAG_SHADER);

        // Sprite setup
        spriteShader.create(SPRITE_VERT_SHADER, SPRITE_FRAG_SHADER);
        spriteShader.bind();
        spriteShader.setInt("image", 0);
        spriteShader.unbind();

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

        GL_CHECK(glGenVertexArrays(1, &spriteVAO));
        GL_CHECK(glGenBuffers(1, &VBO));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        GL_CHECK(glBindVertexArray(spriteVAO));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));  
        GL_CHECK(glBindVertexArray(0));

        
    }

    void clearColor(const ColorRGBAf& color = {0.1f, 0.1f, 0.1f, 1.f})
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void drawTexture(Texture& tex, const Rectf& rect, const float rot = 0, const ColorRGBAf& color = { 1,1,1,1 })
    {
        spriteShader.bind();

        spriteShader.setMat4f("projection", projection);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(rect.x, rect.y, 0.0f));

        model = glm::translate(model, glm::vec3(0.5f * rect.width, 0.5f * rect.height, 0.0f)); 
        model = glm::rotate(model, glm::radians(rot), glm::vec3(0.0f, 0.0f, 1.0f)); 
        model = glm::translate(model, glm::vec3(-0.5f * rect.width, -0.5f * rect.height, 0.0f));

        model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f)); 

        spriteShader.setMat4f("model", model);
        spriteShader.setVec4f("spriteColor", color.r, color.g, color.b, color.a);
        
        tex.bind();
        GL_CHECK(glActiveTexture(GL_TEXTURE0));

        GL_CHECK(glBindVertexArray(spriteVAO));
        GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 6));
        GL_CHECK(glBindVertexArray(0));

        Shader::unbind();
        Texture::unbind();
    }

    void drawLines(const std::vector<Vector2f>& lines, const ColorRGBAf& color, float width = 1, bool loop = false)
    {
        primShader.bind();
        primShader.setMat4f("projection", projection);
        primShader.setVec4f("color", color.r, color.g, color.b, color.a);

        GLuint vao;
        GLuint vbo;
        GL_CHECK(glGenVertexArrays(1, &vao));
        GL_CHECK(glGenBuffers(1, &vbo));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, getVecSizeInBytes(lines), lines.data(), GL_DYNAMIC_DRAW));

        GL_CHECK(glBindVertexArray(vao));
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0));

        glLineWidth(width);
        GL_CHECK(glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, lines.size()));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GL_CHECK(glBindVertexArray(0));

        primShader.unbind();
    }

    void drawRect(int x, int y, int w, int h, const ColorRGBAf& color)
    {
        std::vector<Vector2f> verts(4);
        verts[0] = Vector2f(x,     y);
        verts[1] = Vector2f(x + w, y);
        verts[2] = Vector2f(x + w, y + h);
        verts[3] = Vector2f(x,     y + h);

        drawLines(verts, color, 1, true);
    }

    static inline View getDefaultWindowView(const Window& win)
    {
        const auto winSize = win.getSize();
        return View( 0, 0, winSize.x, winSize.y );
    }

    void setView(const View& v)
    {
        this->view = v;
        projection = v.getMatrix();
    }

    View getView()
    {
        return view;
    }

    bool created() { return window != nullptr; }
};