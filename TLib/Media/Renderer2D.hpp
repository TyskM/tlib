
#pragma once

#include <TLib/DataStructures.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Frustum.hpp>
#include <glm/gtx/rotate_vector.hpp>

struct TextureParams
{
    SharedPtr<Texture> texture;
    SharedPtr<Shader>  shader;
    Rectf      dstrect;
    Rectf      srcrect;
    ColorRGBAf color    = { 1.f, 1.f, 1.f, 1.f };
    float      rotation = 0.f;
    Vector2f   origin  { FLT_MAX, FLT_MAX }; // Defaults to center of sprite
    bool       flipuvx  = false;
    bool       flipuvy  = false;

    TextureParams(const SharedPtr<Texture>& texturev, const Rectf& srcrectv, const Rectf& dstrectv) :
        texture{ texturev }, srcrect{ srcrectv }, dstrect{ dstrectv } { }
};

struct Renderer2D
{
#pragma region Public
public:

    void create(Renderer& renderer)
    {
        this->renderer = &renderer;
        prim_init();
        sprite_init();
    }

    void shutdown() { }

    void begin()
    {

    }

    void render()
    { flushAll(); }

    void setView(const Camera2D& camera)
    {
        auto bounds = camera.getBounds();
        auto mat = camera.getMatrix();
        sprite_shader.setMat4f("projection", mat);
        prim_shader  .setMat4f("projection", mat);
        frustum = Frustum(mat);

        auto fbSize = Renderer::getFramebufferSize();
        glViewport(0, fbSize.y - bounds.height, bounds.width, bounds.height);
    }

    void clearColor(const ColorRGBAf& color = { 0.1f, 0.1f, 0.1f, 1.f })
    { renderer->clearColor(color); }

    #pragma region Sprites

    void drawTexture(const SharedPtr<Texture>& tex, const Rectf& dstrect, float rotation = 0, const ColorRGBAf color = { 1, 1, 1, 1 })
    {
        TextureParams params{ tex, Rectf{ Vector2f{0,0}, Vector2f(tex->getSize()) }, dstrect };
        params.rotation = rotation;
        params.color = color;
        drawTexture(params);
    }

    void drawTexture(const SharedPtr<Texture>& tex, const Rectf& srcrect, const Rectf& dstrect, float rotation = 0, const ColorRGBAf color = { 1, 1, 1, 1 })
    {
        TextureParams params{ tex, srcrect, dstrect };
        params.rotation = rotation;
        params.color = color;
        drawTexture(params);
    }

    void drawTexture(const TextureParams& params)
    { sprite_batch(params); }

    #pragma endregion

    #pragma region Primitives

    void drawLine(const Vector2f& start, const Vector2f& end, const ColorRGBAf& color, float width = 1)
    { prim_batch(std::vector{ start, end }, color, width); }

    void drawRect(float x, float y, float w, float h, const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false, float rot = 0.f)
    {
        std::vector<Vector2f> verts(4);
        verts[0] = Vector2f(x, y);
        verts[1] = Vector2f(x + w, y);
        verts[2] = Vector2f(x + w, y + h);
        verts[3] = Vector2f(x, y + h);

        if (rot != 0)
        {
            for (auto& v : verts)
            {
                v -= {x, y};
                v.rotate(rot);
                v += {x, y};
            }
        }

        prim_batch(verts, color, 1, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop);
    }

    void drawRect(const Rectf& rect, const ColorRGBAf& color = ColorRGBAf::white(), bool filled = false, float rot = 0.f)
    { drawRect(rect.x, rect.y, rect.width, rect.height, color, filled, rot); }

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
                    const ColorRGBAf& color = ColorRGBAf::white(), const bool filled = false, const int segmentCount = 16)
    {
        const float theta = 3.1415926f * 2.f / static_cast<float>(segmentCount);
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
            x += tx * tangetial_factor;
            y += ty * tangetial_factor;
            x *= radial_factor;
            y *= radial_factor;
        }

        prim_batch(points, color, 1.f, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop);
    }

    #pragma endregion

#pragma endregion

#pragma region Impl
private:
    #pragma region Core

    Renderer* renderer = nullptr;
    Frustum frustum;

    enum class RendererType
    {
        Sprite,
        Text,
        Primitive
    };

    void flushAllExcept(RendererType exception)
    {
        if (exception != RendererType::Primitive) { prim_flush();   }
        if (exception != RendererType::Sprite)    { sprite_flush(); }
        if (exception != RendererType::Text)      { text_flush();   }
    }

    void flushAll()
    {
        sprite_flush();
        prim_flush();
        text_flush();
    }

    #pragma endregion

    #pragma region Sprites

    #pragma region Shaders

    String sprite_vertShader = R"(
        #version 330 core

        layout(location = 0) in vec2 vert[4];
        layout(location = 4) in vec2 texCoords[4];
        layout(location = 8) in vec4 color;
        layout(location = 9) in vec3 rot; // x = rot, yz = origin

        out vec2 fragTexCoords;
        out vec4 fragColor;

        uniform mat4 projection;

        vec2 rotated(const vec2 vec, const float radians)
        {
            float sinv = sin(radians);
            float cosv = cos(radians);
            return vec2(vec.x * cosv - vec.y * sinv,
                        vec.x * sinv + vec.y * cosv);
        }

        vec2 getVert()     { return vert[gl_VertexID]; }
        vec2 getTexCoord() { return texCoords[gl_VertexID]; }
        vec4 getColor()    { return color; }

        void main()
        {
            fragTexCoords = getTexCoord();
            fragColor     = getColor();
            
            vec2 vert = getVert() - rot.yz;
            vert      = rotated(vert, rot.x);
            vert     += rot.yz;
            gl_Position = projection * vec4(vert, 0.0, 1.0);
        } )";

    String sprite_fragShader = R"(
        #version 330 core
        in vec2 fragTexCoords;
        in vec4 fragColor;
        out vec4 outColor;

        uniform sampler2D image;

        void main()
        {
            outColor = fragColor * texture(image, fragTexCoords);
        } )";

    #pragma endregion

    struct GLSprite
    {
        std::array<glm::vec2, 4> verts;
        std::array<glm::vec2, 4> texCoords;
        glm::vec4 color;
        glm::vec3 rot; // x = rot, yz = origin
    };

    const std::vector<uint32_t> sprite_indices = { 0, 2, 1, 1, 2, 3 };
    Shader                      sprite_shader;
    std::vector<GLSprite>       sprite_batchBuffer;
    WeakPtr<Texture>            sprite_lastTexture;
    Mesh                        sprite_mesh;

    void sprite_init()
    {
        rendlog->info("Initing sprites...");

        sprite_shader.create(sprite_vertShader, sprite_fragShader);

        Layout layout;
        layout.append(Layout::Vec2f(), 4);
        layout.append(Layout::Vec2f(), 4);
        layout.append(Layout::Vec4f());
        layout.append(Layout::Vec3f());
        layout.setDivisor(1);

        sprite_mesh.setLayout(layout);
        sprite_mesh.setIndices(sprite_indices);

        sprite_batchBuffer.reserve(1024*8);
    }

    void sprite_batch(const TextureParams& cmd)
    {
        flushAllExcept(RendererType::Sprite);

        // Flush early if textures change
        // Use texture atlas to avoid texture changes
        if (cmd.texture != sprite_lastTexture.lock())
        { sprite_flush(); }

        sprite_lastTexture = cmd.texture;

        sprite_batchBuffer.emplace_back();
        GLSprite& sprite = sprite_batchBuffer.back();

        float xpluswidth  = cmd.dstrect.x + cmd.dstrect.width;
        float yplusheight = cmd.dstrect.y + cmd.dstrect.height;

        sprite.verts[0] = { cmd.dstrect.x, cmd.dstrect.y }; // topleft
        sprite.verts[1] = { xpluswidth,    cmd.dstrect.y }; // topright
        sprite.verts[2] = { cmd.dstrect.x, yplusheight   }; // bottom left
        sprite.verts[3] = { xpluswidth,    yplusheight   }; // bottom right

        Vector2f origin = cmd.origin;
        if (origin.x == FLT_MAX) {
            origin = Vector2f(cmd.dstrect.x + cmd.dstrect.width  / 2,
                              cmd.dstrect.y + cmd.dstrect.height / 2);
        }

        sprite.rot = glm::vec3(cmd.rotation, origin.x, origin.y);

        const Vector2f texSize(cmd.texture->getSize());
        float normalWidth  = (cmd.srcrect.x + cmd.srcrect.width)  / texSize.x;
        float normalHeight = (cmd.srcrect.y + cmd.srcrect.height) / texSize.y;
        float normalX      = cmd.srcrect.x / texSize.x;
        float normalY      = cmd.srcrect.y / texSize.y;

        if (cmd.flipuvx)
        {
            auto tmp = normalX;
            normalX = normalWidth;
            normalWidth = tmp;
        }
        if (cmd.flipuvy)
        {
            auto tmp = normalY;
            normalY = normalHeight;
            normalHeight = tmp;
        }

        sprite.texCoords[0] = { normalX,      normalY      }; // topleft
        sprite.texCoords[1] = { normalWidth,  normalY      }; // topright
        sprite.texCoords[2] = { normalX,      normalHeight }; // bottom left
        sprite.texCoords[3] = { normalWidth,  normalHeight }; // bottom right

        sprite.color = { cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a };
    }

    void sprite_flush()
    {
        SharedPtr<Texture> tex = sprite_lastTexture.lock();
        if (!tex) { return; }
        tex->bind();
        sprite_mesh.setData(sprite_batchBuffer, AccessType::Dynamic);
        renderer->drawInstanced(sprite_shader, sprite_mesh, sprite_batchBuffer.size());
        sprite_batchBuffer.clear();
    }

    #pragma endregion

    #pragma region Text

    void text_init()
    {
        rendlog->info("Initing text...");
    }

    void text_batch()
    {

    }

    void text_flush()
    {

    }

    #pragma endregion

    #pragma region Primitives

    #pragma region Shaders
    const char* prim_vertShader = R"""(
    #version 330 core
    layout (location = 0) in vec2 vertex;

    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    }
    )""";

    const char* prim_fragShader = R"""(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 color;

    void main()
    {
       FragColor = color;
    }
    )""";
    #pragma endregion

    static constexpr GLuint prim_restartIndex = std::numeric_limits<GLuint>::max();
    Shader prim_shader;
    Mesh   prim_mesh;
    std::vector<Vector2f> prim_batchBuffer;
    std::vector<GLuint>   prim_batchBufferIndices;

    GLDrawMode prim_lastMode  = GLDrawMode::Unknown;
    ColorRGBAf prim_lastColor = { 1,1,1,1 };
    float      prim_lastWidth = 1;
    

    void prim_init()
    {
        rendlog->info("Initing primitives...");
        prim_shader.create(prim_vertShader, prim_fragShader);
        prim_mesh.setLayout({ Layout::Vec2f() });
    }
    
    void prim_batch(const std::vector<Vector2f>& points, const ColorRGBAf& color = ColorRGBAf::white(),
                    float width = 1, GLDrawMode mode = GLDrawMode::LineStrip)
    {
        flushAllExcept(RendererType::Primitive);

        if (mode  != prim_lastMode  ||
            color != prim_lastColor ||
            width != prim_lastWidth)
        { prim_flush(); }

        prim_lastMode  = mode;
        prim_lastColor = color;
        prim_lastWidth = width;

        for (auto& point : points)
        {
            prim_batchBufferIndices.push_back(static_cast<GLuint>(prim_batchBuffer.size()));
            prim_batchBuffer.push_back(point);
        }
        prim_batchBufferIndices.push_back(prim_restartIndex);
    }

    void prim_flush()
    {
        if (prim_batchBuffer.empty()) { return; }

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(prim_restartIndex);

        prim_shader.setVec4f("color", prim_lastColor.r, prim_lastColor.g, prim_lastColor.b, prim_lastColor.a);
        glLineWidth(prim_lastWidth);

        prim_mesh.setData(prim_batchBuffer, AccessType::Dynamic);
        prim_mesh.setIndices(prim_batchBufferIndices, AccessType::Dynamic);

        RenderState rs;
        rs.drawMode = prim_lastMode;
        Renderer::draw(prim_shader, prim_mesh, rs);

        prim_batchBuffer.clear();
        prim_batchBufferIndices.clear();
    }

    #pragma endregion

#pragma endregion
};