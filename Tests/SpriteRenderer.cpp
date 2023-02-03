//
// Created by Ty on 2023-01-29.
//

#include <TLib/Media/Renderer2D.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include <TLib/Media/Camera2DDebug.hpp>

#include "Common.hpp"

//template <typename... FmtArgs>
//String formatShader(String str, FmtArgs... args)
//{
//    strhelp::replace(str, "{", "{{");
//    strhelp::replace(str, "}", "}}");
//
//    while (true)
//    {
//        if (strhelp::replaceFirst(str, "`", "{"))
//        { strhelp::replaceFirst(str, "`", "}"); }
//        else { break; }
//    }
//
//    return fmt::format(fmt::runtime(str), args...);
//}
//
//struct TextureParams2
//{
//    float      rotation = 0;
//    ColorRGBAf color    = { 1,1,1,1 };
//    bool       flipuvx  = false;
//    bool       flipuvy  = false;
//};
//
//struct TextureRendererOldOld
//{
//protected:
//    String vertShader = R"(
//        #version 330 core
//
//        out vec2 fragTexCoords;
//        out vec4 fragColor;
//
//        uniform mat4 projection;
//
//        vec2 pos[4] = vec2[4](vec2(0.0f,  0.0f),
//                              vec2(1.0f,  0.0f),
//                              vec2(0.0f,  1.0f),
//                              vec2(1.0f,  1.0f));
//
//        struct Sprite
//        {
//            mat4 model;
//            vec4 color;
//            vec2 texCoords[4];
//        };
//
//        layout(std140) uniform Sprites
//        {
//            Sprite sprites[`ubo_size`];
//        };
//
//        vec2 getVert()     { return pos[gl_VertexID]; }
//        mat4 getModel()    { return sprites[gl_InstanceID].model; }
//        vec2 getTexCoord() { return sprites[gl_InstanceID].texCoords[gl_VertexID].xy; }
//        vec4 getColor()    { return sprites[gl_InstanceID].color; }
//
//        void main()
//        {
//            fragTexCoords = getTexCoord();
//            fragColor     = getColor();
//            gl_Position   = projection * getModel() * vec4(getVert(), 0.0, 1.0);
//        } )";
//
//    String fragShader = R"(
//        #version 330 core
//        in vec2 fragTexCoords;
//        in vec4 fragColor;
//        out vec4 outColor;
//
//        uniform sampler2D image;
//
//        void main()
//        {
//            outColor = fragColor * texture(image, fragTexCoords);
//        } )";
//
//    struct Sprite
//    {
//        glm::mat4 model;
//        glm::vec4 color;
//        std::array<glm::vec4, 4> texCoords; // Needs padding bc opengl is terrible
//    };
//
//    const std::vector<uint32_t> indices = { 0, 2, 1, 1, 2, 3 };
//
//    Shader              shader;
//    Frustum             frustum;
//    std::vector<Sprite> batchBuffer;
//    Texture*            lastTexture = nullptr;
//    VertexArray         vbo; // Dummy VBO, because some drivers require that a VAO is bound when drawing.
//    UniformBuffer       ubo;
//
//public:
//    void create()
//    {
//        // GL gives minimum 16KB for UBOs
//        size_t maxBatchSize = 16000 / sizeof(Sprite);
//        tlog::info("Max sprite batch size: {}", maxBatchSize);
//
//        shader.create(
//            formatShader(vertShader, fmt::arg("ubo_size", maxBatchSize)),
//            fragShader
//        );
//
//        ubo.create();
//        vbo.create();
//
//        batchBuffer.reserve(maxBatchSize);
//    }
//
//    void setView(const glm::mat4& viewMatrix)
//    {
//        shader.setMat4f("projection", viewMatrix);
//        frustum = Frustum(viewMatrix);
//    }
//
//    void batchTexture(Texture& texture, const Rectf& srcrect, const Rectf& dstrect, const TextureParams2& texParams = TextureParams2())
//    {
//        if (batchBuffer.size() >= batchBuffer.capacity() ||
//            &texture != lastTexture)
//        { drawBatched(); }
//
//        lastTexture = &texture;
//
//        batchBuffer.emplace_back();
//        Sprite& sprite = batchBuffer.back();
//
//        sprite.model = glm::mat4(1.0f);
//        sprite.model = glm::translate(sprite.model, glm::vec3(dstrect.x, dstrect.y, 0.0f));
//
//        if (texParams.rotation != 0)
//        {
//            sprite.model = glm::translate(sprite.model, glm::vec3(0.5f * dstrect.width, 0.5f * dstrect.height, 0.0f));
//            sprite.model = glm::rotate   (sprite.model, texParams.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
//            sprite.model = glm::translate(sprite.model, glm::vec3(-0.5f * dstrect.width, -0.5f * dstrect.height, 0.0f));
//        }
//
//        sprite.model = glm::scale(sprite.model, glm::vec3(dstrect.width, dstrect.height, 1.f));
//
//        const Vector2f texSize(texture.getSize());
//        float normalWidth  = (srcrect.x + srcrect.width)  / texSize.x;
//        float normalHeight = (srcrect.y + srcrect.height) / texSize.y;
//        float normalX      = srcrect.x / texSize.x;
//        float normalY      = srcrect.y / texSize.y;
//
//        if (texParams.flipuvx)
//        {
//            auto tmp = normalX;
//            normalX = normalWidth;
//            normalWidth = tmp;
//        }
//        if (texParams.flipuvy)
//        {
//            auto tmp = normalY;
//            normalY = normalHeight;
//            normalHeight = tmp;
//        }
//
//        sprite.texCoords[0] = { normalX,      normalY,      0, 0 }; // topleft
//        sprite.texCoords[1] = { normalWidth,  normalY,      0, 0 }; // topright
//        sprite.texCoords[2] = { normalX,      normalHeight, 0, 0 }; // bottom left
//        sprite.texCoords[3] = { normalWidth,  normalHeight, 0, 0 }; // bottom right
//
//        sprite.color = { texParams.color.r, texParams.color.g, texParams.color.b, texParams.color.a };
//    }
//
//    void batchTexture(Texture& texture, const Rectf& dstrect, const TextureParams2& texParams = TextureParams2())
//    { batchTexture(texture, Rectf{ Vector2f(0, 0), Vector2f(texture.getSize()) }, dstrect, texParams); }
//
//    void batchTexture(SubTexture& subtex, const Rectf& dstrect, const TextureParams2& texParams = TextureParams2())
//    { batchTexture(*subtex.texture, Rectf(subtex.rect), dstrect, texParams); }
//
//    void drawBatched()
//    {
//        if (!lastTexture) { return; }
//        lastTexture->bind();
//        vbo.bind();
//        ubo.bind();
//        ubo.bufferData(batchBuffer, AccessType::Dynamic);
//        shader.setUniformBlock("Sprites", ubo, 0);
//        glBlendFunc(static_cast<GLenum>(GLBlendMode::SrcAlpha), static_cast<GLenum>(GLBlendMode::OneMinusSrcAlpha));
//        shader.bind();
//        GL_CHECK(glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices.data(), batchBuffer.size()));
//        batchBuffer.clear();
//    }
//};
//
//struct TextureRendererOld
//{
//protected:
//    String vertShader = R"(
//        #version 330 core
//
//        layout(location = 0) in mat4 model;
//        layout(location = 4) in vec4 color;
//        layout(location = 5) in vec2 texCoords[4];
//
//        out vec2 fragTexCoords;
//        out vec4 fragColor;
//
//        uniform mat4 projection;
//
//        vec2 pos[4] = vec2[4](vec2(0.0f,  0.0f),
//                              vec2(1.0f,  0.0f),
//                              vec2(0.0f,  1.0f),
//                              vec2(1.0f,  1.0f));
//
//
//        vec2 getVert()     { return pos[gl_VertexID]; }
//        mat4 getModel()    { return model; }
//        vec2 getTexCoord() { return texCoords[gl_VertexID].xy; }
//        vec4 getColor()    { return color; }
//
//        void main()
//        {
//            fragTexCoords = getTexCoord();
//            fragColor     = getColor();
//            gl_Position   = projection * getModel() * vec4(getVert(), 0.0, 1.0);
//        } )";
//
//    String fragShader = R"(
//        #version 330 core
//        in vec2 fragTexCoords;
//        in vec4 fragColor;
//        out vec4 outColor;
//
//        uniform sampler2D image;
//
//        void main()
//        {
//            outColor = fragColor * texture(image, fragTexCoords);
//        } )";
//
//    struct Sprite
//    {
//        glm::mat4 model;
//        glm::vec4 color;
//        std::array<glm::vec2, 4> texCoords;
//    };
//
//    const std::vector<uint32_t> indices = { 0, 2, 1, 1, 2, 3 };
//
//    Shader              shader;
//    Frustum             frustum;
//    std::vector<Sprite> batchBuffer;
//    Texture*            lastTexture = nullptr;
//    Mesh                mesh;
//    Renderer*           renderer = nullptr;
//
//public:
//    void create(Renderer& renderer, size_t maxBatchSize = 4096)
//    {
//        this->renderer = &renderer;
//
//        tlog::info("Sprite batch size: {}", maxBatchSize);
//
//        shader.create(
//                formatShader(vertShader, fmt::arg("ubo_size", maxBatchSize)),
//                fragShader
//        );
//
//        Layout layout;
//        layout.append(Layout::Mat4f());
//        layout.append(Layout::Vec4f());
//        layout.append(Layout::Vec2f(), 4);
//        layout.setDivisor(1);
//
//        mesh.setLayout(layout);
//        mesh.setIndices(indices);
//
//        batchBuffer.reserve(maxBatchSize);
//    }
//
//    void setView(const glm::mat4& viewMatrix)
//    {
//        shader.setMat4f("projection", viewMatrix);
//        frustum = Frustum(viewMatrix);
//    }
//
//    void batchTexture(Texture& texture, const Rectf& srcrect, const Rectf& dstrect, const TextureParams2& texParams = TextureParams2())
//    {
//        if (batchBuffer.size() >= batchBuffer.capacity() ||
//            &texture != lastTexture)
//        { drawBatched(); }
//
//        lastTexture = &texture;
//
//        batchBuffer.emplace_back();
//        Sprite& sprite = batchBuffer.back();
//
//        sprite.model = glm::mat4(1.0f);
//        sprite.model = glm::translate(sprite.model, glm::vec3(dstrect.x, dstrect.y, 0.0f));
//
//        if (texParams.rotation != 0)
//        {
//            sprite.model = glm::translate(sprite.model, glm::vec3(0.5f * dstrect.width, 0.5f * dstrect.height, 0.0f));
//            sprite.model = glm::rotate   (sprite.model, texParams.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
//            sprite.model = glm::translate(sprite.model, glm::vec3(-0.5f * dstrect.width, -0.5f * dstrect.height, 0.0f));
//        }
//
//        sprite.model = glm::scale(sprite.model, glm::vec3(dstrect.width, dstrect.height, 1.f));
//
//        const Vector2f texSize(texture.getSize());
//        float normalWidth  = (srcrect.x + srcrect.width)  / texSize.x;
//        float normalHeight = (srcrect.y + srcrect.height) / texSize.y;
//        float normalX      = srcrect.x / texSize.x;
//        float normalY      = srcrect.y / texSize.y;
//
//        if (texParams.flipuvx)
//        {
//            auto tmp = normalX;
//            normalX = normalWidth;
//            normalWidth = tmp;
//        }
//        if (texParams.flipuvy)
//        {
//            auto tmp = normalY;
//            normalY = normalHeight;
//            normalHeight = tmp;
//        }
//
//        sprite.texCoords[0] = { normalX,      normalY     }; // topleft
//        sprite.texCoords[1] = { normalWidth,  normalY     }; // topright
//        sprite.texCoords[2] = { normalX,      normalHeight}; // bottom left
//        sprite.texCoords[3] = { normalWidth,  normalHeight}; // bottom right
//
//        sprite.color = { texParams.color.r, texParams.color.g, texParams.color.b, texParams.color.a };
//    }
//
//    void batchTexture(Texture& texture, const Rectf& dstrect, const TextureParams2& texParams = TextureParams2())
//    { batchTexture(texture, Rectf{ Vector2f(0, 0), Vector2f(texture.getSize()) }, dstrect, texParams); }
//
//    void batchTexture(SubTexture& subtex, const Rectf& dstrect, const TextureParams2& texParams = TextureParams2())
//    { batchTexture(*subtex.texture, Rectf(subtex.rect), dstrect, texParams); }
//
//    void drawBatched()
//    {
//        if (!lastTexture) { return; }
//        lastTexture->bind();
//        mesh.setData(batchBuffer, AccessType::Dynamic);
//        renderer->drawInstanced(shader, mesh, batchBuffer.size());
//        batchBuffer.clear();
//    }
//};

struct TexTest : GameTest
{
    SharedPtr<Texture> tex;
    Renderer2D      rend2d;
    Vector2f        pos    = { 0.1f, 0.1f };
    Vector2f        srcpos = { 0, 0 };
    Camera2D        view;

    bool  rotationEnabled   = true;
    int   spriteCount       = 30;
    float offset            = 16;
    int   newSpriteInterval = 12;

    void create() override
    {
        GameTest::create();
        rend2d.create(renderer);

        tex = makeShared<Texture>();
        tex->loadFromFile("assets/ship.png", TextureFiltering::Nearest);

        auto size = Renderer::getFramebufferSize();
        view.setBounds( Rectf( 0, 0, size.x, size.y) );
        rend2d.setView(view);
    }

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);

        

        debugCamera(view);
        rend2d.setView(view);

        Vector2f mwpos = view.localToWorldCoords(Input::mousePos);

        rend2d.begin();
        rend2d.clearColor();
        rend2d.drawTexture(tex, Rectf{ pos.x, pos.y, 32.f, 32.f });

        static float time = 0.f;
        time += delta;
        int count = spriteCount;
        int sr = sqrt(spriteCount);

        for (int x = 0; x < sr; x++)
        {
            for (int y = 0; y < sr; y++)
            {
                const float rot = rotationEnabled ? sin(time) * x + y : 0;
                const ColorRGBAf color =
                {
                    fmodf(sin(time), 1.f) * (y%16),
                    fmodf(cos(time / 2.f) * (x%12), 1.f),
                    fmodf((time)+x+y, 1.f),
                    1
                };
                const Rectf rect ={ Vector2f(x, y) * offset, Vector2f(32,32) };

                rend2d.drawTexture(tex, rect, rot, color);

                --count;
                if (count == 0) break;
            }
        }

        rend2d.drawCircle(mwpos, 12.f);

        rend2d.render();
        renderer.render();

        imgui.newFrame();
        beginDiagWidgetExt();
        ImGui::Checkbox    ("Rotation enabled", &rotationEnabled);
        ImGui::SliderInt   ("Sprite count", &spriteCount, 1, 100 * 100);
        ImGui::SliderInt   ("New sprite interval", &newSpriteInterval, 0, 128);
        ImGui::SliderFloat ("Sprite offset", &offset, 1.f, 128.f, "%.2f");
        ImGui::End();
        drawDiagWidget(&renderer, &fpslimit);
        imgui.render();

        window.swap();
        fpslimit.wait();
    }
};

int main()
{
    TexTest game;
    game.create();
    game.run();
    return 0;
}