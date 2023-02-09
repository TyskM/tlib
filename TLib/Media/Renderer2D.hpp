
#pragma once

#include <TLib/DataStructures.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/Resource/Font.hpp>
#include <TLib/EASTL.hpp>
#include <EASTL/vector.h>
#include <boost/container/small_vector.hpp>
#include <glm/gtx/rotate_vector.hpp>

enum class RendererType
{
    Sprite    = 1 << 0,
    Text      = 1 << 1,
    Primitive = 1 << 2
}; FLAG_ENUM(RendererType);

struct Renderer2D
{
#pragma region Public
public:

    void create(Renderer& renderer) { this->renderer = &renderer; init(); }
    void shutdown()                 { }
    void render()                   { flush(); }

    void setView(const Camera2D& camera)
    {
        auto bounds = camera.getBounds();
        auto mat    = camera.getMatrix();
        defaultShader.setMat4f("projection", mat);
        textShader   .setMat4f("projection", mat);

        frustum = Frustum(mat);

        auto fbSize = Renderer::getFramebufferSize();
        glViewport(0, fbSize.y - bounds.height, bounds.width, bounds.height);
    }

    void clearColor(const ColorRGBAf& color = { 0.1f, 0.1f, 0.1f, 1.f })
    { renderer->clearColor(color); }

    void drawTexture(      Texture&            tex,
                     const Rectf&              dstrect,
                     const int                 layer    = defaultSpriteLayer,
                     const ColorRGBAf&         color    = { 1, 1, 1, 1 },
                     const float               rotation = 0.f,
                     const Vector2f            origin   = originCenter)
    {
        sprite_batch(tex, Rectf(Vector2f{ 0.f,0.f },
                                Vector2f(tex.getSize())), dstrect, layer, color, rotation, origin);
    }

    void drawTexture(      Texture&            tex,
                     const Rectf&              srcrect,
                     const Rectf&              dstrect,
                     const int                 layer    = defaultSpriteLayer,
                     const ColorRGBAf&         color    = { 1, 1, 1, 1 },
                     const float               rotation = 0.f,
                     const Vector2f            origin   = originCenter)
    {
        sprite_batch(tex, srcrect, dstrect, layer, color, rotation, origin);
    }

    void drawLine(const Vector2f&   start,
                  const Vector2f&   end,
                  const int         layer = defaultPrimitiveLayer,
                  const ColorRGBAf& color = ColorRGBAf::white(),
                  const float       width = 1)
    {
        prim_batch(Vec2fCont{ start, end }, layer, color, width);
    }

    void drawRect(float             x,
                  float             y,
                  float             w,
                  float             h,
                  const int         layer  = defaultPrimitiveLayer,
                  const ColorRGBAf& color  = ColorRGBAf::white(),
                  bool              filled = false,
                  float             rot    = 0.f)
    {
        Vec2fCont verts(4);
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

        prim_batch(verts, layer, color, 1, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop);
    }

    void drawRect(const Rectf&      rect,
                  const int         layer  = defaultPrimitiveLayer,
                  const ColorRGBAf& color  = ColorRGBAf::white(),
                  bool              filled = false,
                  float             rot    = 0.f)
    {
        drawRect(rect.x, rect.y, rect.width, rect.height, layer, color, filled, rot);
    }

    void drawGrid(const Vector2f&   start,
                  const Vector2i&   gridCount,
                  Vector2f          gridSize,
                  const int         layer     = defaultPrimitiveLayer,
                  const ColorRGBAf& color     = ColorRGBAf::white())
    {
        const float targetX = gridCount.x * gridSize.x + start.x;
        const float targetY = gridCount.y * gridSize.y + start.y;

        for (int x = 0; x <= gridCount.x; x++)
        {
            const float startx = x + start.x;
            drawLine(
                Vector2f{ startx * gridSize.x, 0 },
                Vector2f{ startx * gridSize.x, targetY },
                layer, color
            );
        }

        for (int y = 0; y <= gridCount.y; y++)
        {
            const float starty = y + start.y;
            drawLine(
                Vector2f{ 0, starty * gridSize.y },
                Vector2f{ targetX, starty * gridSize.y },
                layer, color
            );
        }
    }

    void drawCircle(const Vector2f&   pos,
                    const float       rad,
                    const int         layer        = defaultPrimitiveLayer,
                    const ColorRGBAf& color        = ColorRGBAf::white(),
                    const bool        filled       = false,
                    const int         segmentCount = 16)
    {
        const float theta = 3.1415926f * 2.f / static_cast<float>(segmentCount);
        const float tangetial_factor = tanf(theta);
        const float radial_factor = cosf(theta);

        float x = rad;
        float y = 0;

        Vec2fCont points;
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

        prim_batch(points, layer, color, 1.f, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop);
    }

    void drawText(const String&     text,
                  Font&             font,
                  const Vector2f&   pos,
                  const int         layer = defaultTextLayer,
                  const ColorRGBAf& color = ColorRGBAf::white(),
                  const float       scale = 1.f)
    {
        text_batch(text, font, pos, layer, color, scale);
    }

#pragma endregion

#pragma region Impl
private:

    #pragma region Shaders
    const char* shaderVertSrc = R"""(
    #version 330 core
    layout (location = 0) in vec4 vertex;
    layout (location = 1) in vec4 color;

    out vec2 fragTexCoords;
    out vec4 fragColor;

    uniform mat4 projection;

    void main()
    {
        gl_Position   = projection * vec4(vertex.xy, 0.0, 1.0);
        fragTexCoords = vertex.zw;
        fragColor     = color;
    }
    )""";

    const char* shaderFragSrc = R"""(
    #version 330 core
    in vec2 fragTexCoords;
    in vec4 fragColor;
    out vec4 outColor;

    uniform sampler2D image;

    void main()
    {
        outColor = fragColor * texture(image, fragTexCoords);
    }
    )""";

    const char* textFragSrc = R"(
    #version 330 core
    in vec2 fragTexCoords;
    in vec4 fragColor;
    out vec4 outColor;

    uniform sampler2D image;

    const float width = 0.45;
    const float edge = 0.1;

    void main()
    {
        float distance = texture(image, fragTexCoords).r;
        float alpha    = smoothstep(width, width + edge, distance);

        outColor = vec4(fragColor.xyz, alpha);
    }
    )";
    #pragma endregion

    struct DrawCmd;
    struct PrimVert;

    using DrawCmdCont = eastl::vector<DrawCmd  >;
    using VerticeCont = eastl::vector<PrimVert >;
    using IndiceCont  = eastl::vector<uint32_t >;
    using Vec4fCont   = eastl::vector<glm::vec4>;
    using Vec2fCont   = eastl::vector<Vector2f >;

    // TODO: Add uniform map to draw commands?
    struct DrawCmd
    {
        int        layer;
        GLDrawMode drawMode;
        Texture*   texture = &whiteTex;
        Shader*    shader  = &defaultShader;
        Vec4fCont  posAndCoords;
        IndiceCont indices;
        ColorRGBAf color;

        bool operator<(const DrawCmd& other)
        {
            if (layer       < other.layer) return true;
            if (other.layer < layer      ) return false;

            if (shader       < other.shader) return true;
            if (other.shader < shader)       return false;

            if (texture       < other.texture) return true;
            if (other.texture < texture)       return false;

            if (static_cast<int>(drawMode)       < static_cast<int>(other.drawMode)) return true;
            if (static_cast<int>(other.drawMode) < static_cast<int>(drawMode)      ) return false;

            return false;
        }
    };

    struct PrimVert
    {
        glm::vec4  vert;
        ColorRGBAf color;
    };

    static inline IndiceCont sprite_indices ={ 0, 2, 1, 1, 2, 3 };
    static inline Texture    whiteTex;
    static inline GLubyte    whiteTexData[1][1][4] =
                             { { {255, 255, 255, 255} } };

    static constexpr GLuint   restartIndex = std::numeric_limits<GLuint>::max();
    static constexpr Vector2f originCenter          = { FLT_MAX, FLT_MAX };
    static constexpr int      defaultSpriteLayer    = 0;
    static constexpr int      defaultPrimitiveLayer = 1;
    static constexpr int      defaultTextLayer      = 2;

    static inline Mesh   mesh;
    static inline Shader defaultShader;
    static inline Shader textShader;

    Renderer* renderer = nullptr;
    Frustum   frustum;
    DrawCmdCont drawCmds;
    VerticeCont batchBuffer;
    IndiceCont  batchBufferIndices;

    void init()
    {
        rendlog->info("Initing Renderer2D...");

        if (!mesh.valid())
        { mesh.setLayout({ Layout::Vec4f(), Layout::Vec4f() }); }

        if (!whiteTex.created())
        { whiteTex.setData(whiteTexData, 1, 1, TextureFiltering::Nearest); }

        if (!defaultShader.created())
        { defaultShader.create(shaderVertSrc, shaderFragSrc); }

        if (!textShader.created())
        { textShader.create(shaderVertSrc, textFragSrc); }
    }

    void flushCurrent(Shader* shader, Texture* tex, GLDrawMode drawMode)
    {
        if (batchBuffer.empty()) { return; }

        tex->bind();
        shader->bind();

        mesh.setData(batchBuffer, AccessType::Dynamic);
        mesh.setIndices(batchBufferIndices, AccessType::Dynamic);

        RenderState rs;
        rs.drawMode = drawMode;
        Renderer::draw(*shader, mesh, rs);

        batchBuffer.clear();
        batchBufferIndices.clear();
    }

    void flush()
    {
        if (drawCmds.empty()) { return; }

        std::sort(drawCmds.begin(), drawCmds.end());

        Texture*   lastTexture  = drawCmds[0].texture;
        Shader*    lastShader   = drawCmds[0].shader;
        GLDrawMode lastDrawMode = drawCmds[0].drawMode;

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(restartIndex);

        bool   stateChanged = true;
        size_t offset = 0;

        for (auto& cmd : drawCmds)
        {
            stateChanged = (cmd.texture != lastTexture || cmd.shader != lastShader || cmd.drawMode != lastDrawMode);
            if (stateChanged)
            { flushCurrent(lastShader, lastTexture, lastDrawMode); }

            offset = batchBuffer.size();
            for (auto& ind : cmd.indices)
            { batchBufferIndices.push_back(offset + ind); }
            batchBufferIndices.push_back(restartIndex);

            for (auto& pac : cmd.posAndCoords)
            { batchBuffer.emplace_back(pac, cmd.color); }

            lastTexture  = cmd.texture;
            lastShader   = cmd.shader;
            lastDrawMode = cmd.drawMode;
        }

        flushCurrent(lastShader, lastTexture, lastDrawMode);
        drawCmds.clear();
    }

    void rotate(float& x, float& y, float radians)
    {
        float sinv = sin(radians);
        float cosv = cos(radians);
        float xcopy = x;
        float ycopy = y;
        x = xcopy * cosv - ycopy * sinv;
        y = xcopy * sinv + ycopy * cosv;
    }

    void sprite_batch(      Texture&    texture,
                      const Rectf&      srcrect,
                      const Rectf&      dstrect,
                      const int         layer    = 0,
                      const ColorRGBAf& color    = { 1, 1, 1, 1 },
                      const float       rotation = 0.f,
                      Vector2f          origin   = originCenter,
                      const bool        flipuvx  = false,
                      const bool        flipuvy  = false,
                      Shader&           shader   = defaultShader)
    {
        drawCmds.emplace_back();
        DrawCmd& cmd = drawCmds.back();

        cmd.texture  = &texture;
        cmd.shader   = &shader;
        cmd.layer    = layer;
        cmd.indices  = sprite_indices;
        cmd.drawMode = GLDrawMode::Triangles;
        cmd.color    = color;

        cmd.posAndCoords.resize(4);
        float xpluswidth  = dstrect.x + dstrect.width;
        float yplusheight = dstrect.y + dstrect.height;

        const Vector2f texSize(texture.getSize());
        float normalWidth  = (srcrect.x + srcrect.width)  / texSize.x;
        float normalHeight = (srcrect.y + srcrect.height) / texSize.y;
        float normalX      =  srcrect.x / texSize.x;
        float normalY      =  srcrect.y / texSize.y;

        if (flipuvx)
        {
            auto tmp = normalX;
            normalX = normalWidth;
            normalWidth = tmp;
        }
        if (flipuvy)
        {
            auto tmp = normalY;
            normalY = normalHeight;
            normalHeight = tmp;
        }

        cmd.posAndCoords[0] = { dstrect.x , dstrect.y  , normalX,      normalY      }; // topleft
        cmd.posAndCoords[1] = { xpluswidth, dstrect.y  , normalWidth,  normalY      }; // topright
        cmd.posAndCoords[2] = { dstrect.x , yplusheight, normalX,      normalHeight }; // bottom left
        cmd.posAndCoords[3] = { xpluswidth, yplusheight, normalWidth,  normalHeight }; // bottom right

        if (origin.x == FLT_MAX) {
            origin = Vector2f(dstrect.x + dstrect.width  / 2,
                              dstrect.y + dstrect.height / 2); }
        if (rotation != 0)
        {
            for (auto& v : cmd.posAndCoords)
            {
                v.x -= origin.x; v.y -= origin.y;
                rotate(v.x, v.y, rotation);
                v.x += origin.x; v.y += origin.y;
            }
        }
    }


    void prim_batch(const Vec2fCont&  points,
                    const int         layer = defaultPrimitiveLayer,
                    const ColorRGBAf& color = ColorRGBAf::white(),
                    const float       width = 1,
                    const GLDrawMode  mode  = GLDrawMode::LineStrip)
    {
        drawCmds.emplace_back();
        DrawCmd& cmd = drawCmds.back();

        cmd.texture  = &whiteTex;
        cmd.layer    = layer;

        ASSERT(points.size() > 0);
        cmd.posAndCoords.reserve(points.size());
        cmd.indices.reserve(points.size());

        for (int i = 0; i < points.size(); i++)
        {
            const Vector2f& p = points[i];
            cmd.posAndCoords.emplace_back(p.x, p.y, 0.f, 0.f);
            cmd.indices.push_back(i);
        }
        
        cmd.drawMode = mode;
        cmd.color    = color;
    }

    void text_batch(const String&     text,
                          Font&       font,
                    const Vector2f&   pos,
                    const int         layer = defaultTextLayer,
                    const ColorRGBAf& color = ColorRGBAf::white(),
                    const float       scale = 1.f)
    {
        Vector2f currentPos = pos;
        for (auto& strchar : text)
        {
            // TODO: add default unknown character, and use that instead
            if (!font.containsChar(strchar))
            { std::cout << "Font does not contain character \"" << strchar << "\"" << std::endl; continue; }

            FontCharacter& ch = font.getChar(strchar);
            float w    = ch.size.x * scale;
            float h    = ch.size.y * scale;
            float xpos = (currentPos.x + ch.bearing.x);
            float ypos = (currentPos.y - h + (h - ch.bearing.y * scale));
            currentPos.x += (ch.advance >> 6) * scale;

            sprite_batch(ch.texture,
                         { Vector2f(0,0), Vector2f(ch.texture.getSize()) },
                         { xpos, ypos, w, h },
                         layer,
                         color,
                         0.0f,
                         originCenter,
                         false,
                         false,
                         textShader);

        }
    }

};
