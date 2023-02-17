
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
#include <glm/gtx/rotate_vector.hpp>
#include <span>

using namespace eastl;

namespace rend2Ddetail
{
    struct RendererCallback
    { static int SDLEventFilterCB(void* userdata, SDL_Event* event); };
}

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

    bool created()                  { return this->renderer != nullptr; }
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

        view = camera;
    }

    [[nodiscard]]
    inline Camera2D getView()
    { return view; }

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
        Vector2f line[2] = { start, end };
        prim_batch(line, layer, color, width);
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
        Vector2f verts[4] = {
            Vector2f(x, y),
            Vector2f(x + w, y),
            Vector2f(x + w, y + h),
            Vector2f(x, y + h)
        };

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

    // TODO: Remove heap alloc
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

        vector<Vector2f, MiAllocator> points;
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

    uniform float width = 0.45;
    uniform float edge  = 0.1;

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

    using Allocator  = MiAllocator;
    using IndiceCont = vector<uint32_t , Allocator>;

    // TODO: Add uniform map to draw commands?
    struct DrawCmd
    {
        int        layer;
        GLDrawMode drawMode;
        Texture*   texture = &whiteTex;
        Shader*    shader  = &defaultShader;
        uint32_t   posIndex, posSize; // Index and size for posAndCoords
        uint32_t   indIndex, indSize; // Index and size for indices
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
    Camera2D view;

    Renderer* renderer = nullptr;

    // TODO: actually use the frustum
    // need to calculate AABB of the sprites rect to check collision
    // checking for primitives isn't necessary
    Frustum frustum;

    // Draw data goes here, then is sorted
    vector<DrawCmd, Allocator> drawCmds;

    // Draw data vertex data in these two
    // These aren't stored in the DrawCmd struct so the alloced space can be reused
    vector<glm::vec4, Allocator> posAndCoords;
    IndiceCont                   indices;

    // These buffers are copied to the GPU
    vector<PrimVert, Allocator> batchBuffer;
    IndiceCont                  batchBufferIndices;

    void init()
    {
        if (!mesh.valid())
        { mesh.setLayout({ Layout::Vec4f(), Layout::Vec4f() }); }

        if (!whiteTex.created())
        { whiteTex.setData(whiteTexData, 1, 1); }

        if (!defaultShader.created())
        { defaultShader.create(shaderVertSrc, shaderFragSrc); }

        if (!textShader.created())
        { textShader.create(shaderVertSrc, textFragSrc); }

        Camera2D startCam;
        auto size = Renderer::getFramebufferSize();
        startCam.setBounds(Rectf(0, 0, size.x, size.y));
        setView(startCam);

        SDL_AddEventWatch(&rend2Ddetail::RendererCallback::SDLEventFilterCB, this);

        size_t reserveSize = 1024 * 5;
        drawCmds            .reserve(reserveSize);
        posAndCoords        .reserve(reserveSize);
        indices             .reserve(reserveSize);
        batchBuffer         .reserve(reserveSize);
        batchBufferIndices  .reserve(reserveSize);
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

            for (uint32_t i = cmd.indIndex; i < cmd.indIndex + cmd.indSize; i++)
            { batchBufferIndices.push_back(offset + indices[i]); }

            batchBufferIndices.push_back(restartIndex);

            for (uint32_t i = cmd.posIndex; i < cmd.posIndex + cmd.posSize; i++)
            { batchBuffer.emplace_back(posAndCoords[i], cmd.color); }

            lastTexture  = cmd.texture;
            lastShader   = cmd.shader;
            lastDrawMode = cmd.drawMode;
        }

        flushCurrent(lastShader, lastTexture, lastDrawMode);
        drawCmds.clear();
        posAndCoords.clear();
        indices.clear();
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
        cmd.drawMode = GLDrawMode::Triangles;
        cmd.color    = color;

        cmd.indIndex = indices.size();
        cmd.indSize  = sprite_indices.size();
        indices.insert(indices.end(), sprite_indices.begin(), sprite_indices.end());

        cmd.posIndex = posAndCoords.size();
        cmd.posSize  = 4;


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

        posAndCoords.emplace_back( dstrect.x , dstrect.y  , normalX,      normalY      );  // topleft
        posAndCoords.emplace_back( xpluswidth, dstrect.y  , normalWidth,  normalY      );  // topright
        posAndCoords.emplace_back( dstrect.x , yplusheight, normalX,      normalHeight );  // bottom left
        posAndCoords.emplace_back( xpluswidth, yplusheight, normalWidth,  normalHeight );  // bottom right

        if (rotation != 0)
        {
            if (origin.x == FLT_MAX) {
                origin = Vector2f(dstrect.x + dstrect.width  / 2,
                                  dstrect.y + dstrect.height / 2);
            }

            for (size_t i = posAndCoords.size() - 4; i < posAndCoords.size(); i++)
            {
                auto& v = posAndCoords[i];
                v.x -= origin.x; v.y -= origin.y;
                rotate(v.x, v.y, rotation);
                v.x += origin.x; v.y += origin.y;
            }
        }
    }


    void prim_batch(const std::span<Vector2f>&  points,
                    const int                   layer = defaultPrimitiveLayer,
                    const ColorRGBAf&           color = ColorRGBAf::white(),
                    const float                 width = 1,
                    const GLDrawMode            mode  = GLDrawMode::LineStrip)
    {
        ASSERT(points.size() > 0);

        drawCmds.emplace_back();
        DrawCmd& cmd = drawCmds.back();
        
        cmd.texture  = &whiteTex;
        cmd.layer    = layer;
        cmd.drawMode = mode;
        cmd.color    = color;

        cmd.indIndex = indices.size();
        cmd.indSize  = points.size();
        cmd.posIndex = posAndCoords.size();
        cmd.posSize  = cmd.indSize;
        
        for (int i = 0; i < cmd.indSize; i++)
        {
            const Vector2f& p = points[i];
            posAndCoords.emplace_back(p.x, p.y, 0.f, 0.f);
            indices.push_back(i);
        }
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
            if (strchar == '\n')
            {
                currentPos.y += font.lineHeight();
                currentPos.x = pos.x;
                continue;
            }

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

    void onWindowResized()
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        auto fbSize = Renderer::getFramebufferSize();
        // glViewport origin is bottom left, so i make it top left :))
        glViewport(0, fbSize.y - viewport[3], viewport[2], viewport[3]);
    }

    friend class rend2Ddetail::RendererCallback;
};

namespace rend2Ddetail
{
    int RendererCallback::SDLEventFilterCB(void* userdata, SDL_Event* event)
    {
        Renderer2D* r = static_cast<Renderer2D*>(userdata);
        if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        { r->onWindowResized(); }
        return 1;
    }
}