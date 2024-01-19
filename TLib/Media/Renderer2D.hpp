
#pragma once

#include <TLib/DataStructures.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/Resource/Font.hpp>
#include <TLib/EASTL.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Media/RenderTarget.hpp>
#include <TLib/Embed/Embed.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <span>

struct Renderer2DOrigin
{
    Renderer2DOrigin() = default;
    Renderer2DOrigin(const Vector2f& pos, bool useWorldCoords = false) : pos{pos}, useWorldCoords{useWorldCoords} { }

    // Uses local coords if this is false
    bool useWorldCoords = false;

    Vector2f pos = { FLT_MAX, FLT_MAX };
};

struct Renderer2D
{
#pragma region Public
public:
    static constexpr Vector2f OriginCenter          = { FLT_MAX, FLT_MAX };
    static constexpr int      DefaultSpriteLayer    = 0;
    static constexpr int      DefaultPrimitiveLayer = 1;
    static constexpr int      DefaultTextLayer      = 2;

    static bool created()      { return inited; }
    static void create()       { init();        }

    /*
    @param sort If false, Ignore layer parameter and draw everthing back to front
    @param ignoreCamera If true, will render using the default camera. Useful for drawing UI and FBOs
    */
    static void render(bool sort = false, bool ignoreCamera = false)
    {
        if (ignoreCamera)
        {
            auto oldView = currentView;
            Renderer2D::resetView();
            flush(sort);
            currentView = oldView;
        }
        else
        {
            flush(sort);
        }
    }

    static void setView(const View& view)
    {
        // Projection uniform for shader is set in flushCurrent()

        // TODO: Frustum is unused for now
        //auto mat = camera.getMatrix();
        //frustum  = Frustum(mat);

        Vector2i viewportSize = Vector2i(view.viewport.width, view.viewport.height);
        const Vector2f fbSize(Renderer::getFramebufferSize());

        Renderer::setViewport(getViewportSizePixels(view, fbSize), fbSize);
        currentView = view;
    }

    [[nodiscard]]
    static inline View getView()
    { return currentView; }

    static void resetView()
    {
        setView(getDefaultCamera());
    }

    static View getDefaultCamera()
    {
        View v;
        Vector2f size(Renderer::getFramebufferSize());
        v.size   = size;
        v.center = size / 2.f;
        return v;
    }

    static Vector2f getMouseWorldPos()
    {
        return localToWorldPos(Vector2f(Input::mousePos));
    }

    static Vector2f localToWorldPos(const Vector2f& pos)
    {
        return localToWorldPoint(pos, Renderer2D::getView(), Renderer::getFramebufferSize());
    }

    static void clearColor(const ColorRGBAf& color = { 0.1f, 0.1f, 0.1f, 1.f })
    { Renderer::clearColor(color); }

    // Rotation is in radians
    static void drawTexture(      Texture&            tex,
                            const Rectf&              dstrect,
                            const float               rotation = 0.f,
                            const ColorRGBAf&         color    = { 1.f, 1.f, 1.f, 1.f },
                            const int                 layer    = DefaultSpriteLayer,
                            const Renderer2DOrigin&   origin   = OriginCenter,
                            bool flipUvX                       = false,
                            bool flipUvY                       = false,
                            Shader& shader                     = defaultShader)
    {
        sprite_batch(tex, Rectf(Vector2f{ 0.f,0.f }, Vector2f(tex.getSize())), dstrect,
            rotation,
            color,
            layer,
            origin,
            flipUvX, flipUvY, shader);
    }

    // Rotation is in radians
    static void drawTexture(const SubTexture&         subTex,
                            const Rectf&              dstrect,
                            const float               rotation = 0.f,
                            const ColorRGBAf&         color    = { 1.f, 1.f, 1.f, 1.f },
                            const int                 layer    = DefaultSpriteLayer,
                            const Renderer2DOrigin&   origin   = OriginCenter,
                            bool flipUvX                       = false,
                            bool flipUvY                       = false,
                            Shader& shader                     = defaultShader)
    {
        ASSERT(subTex.texture);
        sprite_batch(*subTex.texture, Rectf(subTex.rect), dstrect,
            rotation,
            color,
            layer,
            origin,
            flipUvX, flipUvY, shader);
    }

    // Rotation is in radians
    static void drawTexture(const SubTexture&         subTex,
                            const Vector2f&           pos,
                            const float               rotation = 0.f,
                            const Vector2f&           scale    = { 1.f, 1.f },
                            const ColorRGBAf&         color    = { 1.f, 1.f, 1.f, 1.f },
                            const int                 layer    = DefaultSpriteLayer,
                            const Renderer2DOrigin&   origin   = OriginCenter,
                            bool flipUvX                       = false,
                            bool flipUvY                       = false,
                            Shader& shader                     = defaultShader)
    {
        ASSERT(subTex.texture);
        Vector2f texSize     = Vector2f(subTex.rect.getSize()) * scale;
        Vector2f halfTexSize = texSize/2.f;

        sprite_batch(*subTex.texture, subTex.rect, Rectf(pos - halfTexSize, texSize),
            rotation,
            color,
            layer,
            origin,
            flipUvX, flipUvY, shader);
    }

    // Rotation is in radians
    static void drawTexture(        Texture&            tex,
                            const   Rectf&              srcrect,
                            const   Rectf&              dstrect,
                            const   float               rotation = 0.f,
                            const   ColorRGBAf&         color    = { 1.f, 1.f, 1.f, 1.f },
                            const   int                 layer    = DefaultSpriteLayer,
                            const   Renderer2DOrigin&   origin   = OriginCenter,
                            bool    flipUvX                      = false,
                            bool    flipUvY                      = false,
                            Shader& shader                       = defaultShader)
    {
        sprite_batch(tex, srcrect, dstrect,
            rotation,
            color,
            layer,
            origin,
            flipUvX, flipUvY, shader);
    }

    // Rotation is in radians
    static void drawTexture(        Texture&            tex,
                            const   Rectf&              srcrect,
                            const   Vector2f&           pos,
                            const   float               rotation = 0.f,
                            const   Vector2f&           scale    = { 1.f, 1.f },
                            const   ColorRGBAf&         color    = { 1.f, 1.f, 1.f, 1.f },
                            const   int                 layer    = DefaultSpriteLayer,
                            const   Renderer2DOrigin&   origin   = OriginCenter,
                            bool    flipUvX                      = false,
                            bool    flipUvY                      = false,
                            Shader& shader                       = defaultShader)
    {
        Vector2f texSize     = Vector2f(tex.getSize()) * scale;
        Vector2f halfTexSize = texSize/2.f;

        sprite_batch(tex, srcrect, Rectf(pos - halfTexSize, texSize),
            rotation,
            color,
            layer,
            origin,
            flipUvX, flipUvY, shader);
    }

    // Rotation is in radians
    static void drawTexture(      Texture&            tex,
                            const Vector2f&           pos,
                            const float               rotation = 0.f,
                            const Vector2f&           scale    = { 1.f, 1.f },
                            const ColorRGBAf&         color    = { 1.f, 1.f, 1.f, 1.f },
                            const int                 layer    = DefaultSpriteLayer,
                            const Renderer2DOrigin&   origin   = OriginCenter,
                            bool flipUvX = false,
                            bool flipUvY = false,
                            Shader& shader = defaultShader)
    {
        drawTexture(
            tex,
            Rectf(Vector2f{0.f,0.f}, Vector2f(tex.getSize())),
            pos,
            rotation,
            scale,
            color,
            layer,
            origin,
            flipUvX, flipUvY, shader);
    }

    static void drawRenderTarget(RenderTarget&           target,
                                 const Rectf&            srcrect,
                                 const Rectf&            dstrect,
                                 const float             rotation = 0.f,
                                 const ColorRGBAf&       color    = {1.f, 1.f, 1.f, 1.f},
                                 const int               layer    = DefaultSpriteLayer,
                                 const Renderer2DOrigin& origin   = OriginCenter,
                                 bool                    flipUvX  = false,
                                 bool                    flipUvY  = false,
                                 Shader&                 shader   = defaultShader)
    {
        // Render target textures need to be flipped on the Y,
        // because openGL is weird and uses bottom-left as the origin.
        drawTexture(target.texture, srcrect, dstrect, rotation, color, layer, origin, flipUvX, !flipUvY, shader);
    }

    static void drawRenderTarget(RenderTarget&          target,
                                const Rectf&            dstrect,
                                const float             rotation = 0.f,
                                const ColorRGBAf&       color    = {1.f, 1.f, 1.f, 1.f},
                                const int               layer    = DefaultSpriteLayer,
                                const Renderer2DOrigin& origin   = OriginCenter,
                                bool                    flipUvX  = false,
                                bool                    flipUvY  = false,
                                Shader&                 shader   = defaultShader)
    {
        drawTexture(target.texture, dstrect, rotation, color, layer, origin, flipUvX, !flipUvY, shader);
    }

    static void drawFinalRenderTarget(RenderTarget& rt)
    {
        Renderer2D::setView(rt.view);
        Renderer::setViewport(rt.getViewportSizePixels(), Vector2f(rt.getSize()));
        Renderer2D::drawRenderTarget(rt, Rectf(0, 0, Vector2f(rt.texture.getSize())));
    }

    static void bindRenderTarget(RenderTarget& rt)
    {
        ASSERT(rt.created());
        rt.bind();
        Renderer2D::setView(rt.view);
        Renderer::setViewport(rt.getViewportSizePixels(), Vector2f(rt.getSize()));
    }

    /*
    Draws an image using 9(slice/grid/patch) scaling
    https://en.wikipedia.org/wiki/9-slice_scaling

    If your (dstRect.width < left + right || dstRect.height < top + bottom)
    your image will probably look weird
    */
    static void drawNinePatchTex(
            Texture& tex,
        const Rectf& srcRect,
        const Rectf& dstRect,
        float left, float right, float top, float bottom)
    {
        // What a process zzzzzzzzz
        const Vector2f& totalSize = Vector2f(srcRect.getSize());

        Vector2f centerSize = totalSize;
        centerSize.x -= left + right;
        centerSize.y -= top + bottom;
        Vector2f centerPos = srcRect.getPos() + Vector2f{left, top};
        Rectf centerSrc ={centerPos, centerSize};

        Rectf mtSrc ={srcRect.getPos() + Vector2f(left,                0), centerSize.x, top};
        Rectf mlSrc ={srcRect.getPos() + Vector2f(0,                   top), left,         centerSize.y};
        Rectf mrSrc ={srcRect.getPos() + Vector2f(totalSize.x - right, top), right,        centerSize.y};
        Rectf mbSrc ={srcRect.getPos() + Vector2f(left,                totalSize.y - bottom), centerSize.x, bottom};
        Rectf tlSrc ={srcRect.getPos() + Vector2f(0,                   0), left,         right};
        Rectf trSrc ={srcRect.getPos() + Vector2f(totalSize.x - right, 0), right,        top};
        Rectf blSrc ={srcRect.getPos() + Vector2f(0,                   totalSize.y - bottom), left,         bottom};
        Rectf brSrc ={srcRect.getPos() + Vector2f(totalSize.x - right, totalSize.y - bottom), right,        bottom};

        Rectf centerDst ={dstRect.x + left, dstRect.y + top, dstRect.width - right - left, dstRect.height - bottom - top};

        Rectf mtDst ={dstRect.x + left,           dstRect.y,                    centerDst.width, top};
        Rectf mlDst ={dstRect.x,                  dstRect.y + top,              left,            centerDst.height};
        Rectf mrDst ={dstRect.getRight() - right, dstRect.y + top,              right,           centerDst.height};
        Rectf mbDst ={dstRect.x + left,           dstRect.getBottom() - bottom, centerDst.width, bottom};

        Rectf tlDst ={dstRect.getPos(),                           tlSrc.getSize()};
        Rectf trDst ={dstRect.getRight() - right,                 dstRect.y,                    trSrc.getSize()};
        Rectf blDst ={dstRect.x,                                  dstRect.getBottom() - bottom, blSrc.getSize()};
        Rectf brDst ={dstRect.getRightBottom() - brSrc.getSize(), brSrc.getSize()};

        drawTexture(tex, mtSrc, mtDst); // Draw sides
        drawTexture(tex, mlSrc, mlDst);
        drawTexture(tex, mrSrc, mrDst);
        drawTexture(tex, mbSrc, mbDst);
        drawTexture(tex, tlSrc, tlDst); // Then corners
        drawTexture(tex, trSrc, trDst);
        drawTexture(tex, blSrc, blDst);
        drawTexture(tex, brSrc, brDst);
        drawTexture(tex, centerSrc, centerDst); // Then center
    }

    static void drawNinePatchTex(
            Texture& tex,
        const Rectf& dstRect,
        float left, float right, float top, float bottom)
    {
        drawNinePatchTex(tex, Rectf(0.f, 0.f, Vector2f(tex.getSize())), dstRect, left, right, top, bottom);
    }

    // loop: will draw a line between first and last point
    static void drawLines(const std::span<const Vector2f>& points,
                          const ColorRGBAf&                color    = ColorRGBAf::white(),
                          GLDrawMode                       drawMode = GLDrawMode::Lines,
                          const int                        layer    = DefaultPrimitiveLayer)
    {
        prim_batch(points, color, drawMode, layer);
    }

    static void drawLine(const Vector2f&   start,
                         const Vector2f&   end,
                         const ColorRGBAf& color = ColorRGBAf::white(),
                         const int         layer = DefaultPrimitiveLayer)
    {
        Vector2f line[2] = { start, end };
        prim_batch(line, color, GLDrawMode::LineStrip, layer);
    }

    static void drawRect(float             x,
                         float             y,
                         float             w,
                         float             h,
                         float             rot    = 0.f,
                         bool              filled = false,
                         const ColorRGBAf& color  = ColorRGBAf::white(),
                         const int         layer  = DefaultPrimitiveLayer)
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

        prim_batch(verts, color, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop, layer);
    }

    static void drawRect(const Rectf&      rect,
                         float             rot    = 0.f,
                         bool              filled = false,
                         const ColorRGBAf& color  = ColorRGBAf::white(),
                         const int         layer  = DefaultPrimitiveLayer)
    {
        drawRect(rect.x, rect.y, rect.width, rect.height, rot, filled, color, layer);
    }

    static void drawGrid(const Vector2f&   start,
                         const Vector2i&   gridCount,
                         Vector2f          gridSize,
                         const ColorRGBAf& color     = ColorRGBAf::white(),
                         const int         layer     = DefaultPrimitiveLayer)
    {
        const float targetX = gridCount.x * gridSize.x + start.x;
        const float targetY = gridCount.y * gridSize.y + start.y;

        for (int x = 0; x <= gridCount.x; x++)
        {
            const float startx = x + start.x;
            drawLine(
                Vector2f{ startx * gridSize.x, 0 },
                Vector2f{ startx * gridSize.x, targetY },
                color, layer
            );
        }

        for (int y = 0; y <= gridCount.y; y++)
        {
            const float starty = y + start.y;
            drawLine(
                Vector2f{ 0, starty * gridSize.y },
                Vector2f{ targetX, starty * gridSize.y },
                color, layer
            );
        }
    }

    // TODO: Remove heap alloc
    static void drawCircle(const Vector2f&   pos,
                           const float       rad,
                           const bool        filled       = false,
                           const ColorRGBAf& color        = ColorRGBAf::white(),
                           const int         segmentCount = 16,
                           const int         layer        = DefaultPrimitiveLayer)
    {
        const float theta = 3.1415926f * 2.f / static_cast<float>(segmentCount);
        const float tangetial_factor = tanf(theta);
        const float radial_factor = cosf(theta);

        float x = rad;
        float y = 0;

        Vector<Vector2f> points;
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

        prim_batch(points, color, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop, layer);
    }

    static void drawText(const String&     text,
                         Font&             font,
                         const Vector2f&   pos,
                         const ColorRGBAf& color = ColorRGBAf::white(),
                         const float       scale = 1.f,
                         const int         layer = DefaultTextLayer)
    {
        text_batch(text, font, pos, layer, color, scale);
    }

    static inline void setSDFTextWidth(const float width)
    {
        sdfTextWidth = width;
        textShader.setFloat("width", width);
    }

    static inline void setSDFTextEdge(const float edge)
    {
        sdfTextEdge = edge;
        textShader.setFloat("edge", edge);
    }

    static inline float getSDFTextWidth()
    { return sdfTextWidth; }

    static inline float getSDFTextEdge()
    { return sdfTextEdge; }

    static inline Vector2f getRectDefaultOrigin(const Rectf& rect)
    {
        return Vector2f(rect.x + rect.width  / 2,
                        rect.y + rect.height / 2);
    }

#pragma endregion

#pragma region Impl
private:
    using IndiceCont = Vector<uint32_t>;

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

    static inline Mesh      mesh;
    static inline Shader    defaultShader;
    static inline Shader    textShader;
    static inline View      currentView;
    static inline bool      inited = false;

    static inline float sdfTextWidth;
    static inline float sdfTextEdge;

    // TODO: actually use the frustum
    // need to calculate AABB of the sprites rect to check collision
    // checking for primitives isn't necessary
    static inline Frustum frustum;

    // Draw data goes here, then is sorted
    static inline Vector<DrawCmd> drawCmds;

    // Draw data vertex data in these two
    // These aren't stored in the DrawCmd struct so the alloced space can be reused
    static inline Vector<glm::vec4> posAndCoords;
    static inline IndiceCont        indices;

    // These buffers are copied to the GPU
    static inline Vector<PrimVert> batchBuffer;
    static inline IndiceCont       batchBufferIndices;

    static int SDLEventFilterCB(void* userdata, SDL_Event* event)
    {
        if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        { Renderer2D::onWindowResized(); }
        return 0;
    }

    static void init()
    {
        if (inited) { return; }

        if (!Renderer::created())
        { Renderer::create(); }

        if (!mesh.valid())
        { mesh.setLayout({ Layout::Vec4f(), Layout::Vec4f() }); }

        if (!whiteTex.created())
        { whiteTex.setData(whiteTexData, 1, 1); }

        if (!defaultShader.created())
        {
            defaultShader.create( myEmbeds["TLib/Embed/Shaders/2d.vert"].asString().c_str(),
                                  myEmbeds["TLib/Embed/Shaders/2d.frag"].asString().c_str());
        }

        if (!textShader.created())
        {
            textShader.create( myEmbeds["TLib/Embed/Shaders/2d.vert"].asString().c_str(),
                               myEmbeds["TLib/Embed/Shaders/sdf_text.frag"].asString().c_str());
        }

        setSDFTextEdge(0.04f);
        setSDFTextWidth(0.48f);

        resetView();

        SDL_AddEventWatch(&SDLEventFilterCB, NULL);

        size_t reserveSize = size_t(1024) * 5;
        drawCmds            .reserve(reserveSize);
        posAndCoords        .reserve(reserveSize);
        indices             .reserve(reserveSize);
        batchBuffer         .reserve(reserveSize);
        batchBufferIndices  .reserve(reserveSize);
        inited = true;
    }

    static void flushCurrent(Shader* shader, Texture* tex, GLDrawMode drawMode)
    {
        if (batchBuffer.empty()) { return; }

        tex->bind();
        shader->bind();

        shader->setMat4f("projection", currentView.getMatrix());
        mesh.setData(batchBuffer, AccessType::Dynamic);
        mesh.setIndices(batchBufferIndices, AccessType::Dynamic);

        RenderState rs;
        rs.drawMode = drawMode;
        Renderer::draw(*shader, mesh, rs);

        batchBuffer.clear();
        batchBufferIndices.clear();
    }

    static void flush(bool sort)
    {
        if (drawCmds.empty()) { return; }

        if (sort)
        { std::sort(drawCmds.begin(), drawCmds.end()); }

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

    static void rotate(float& x, float& y, float radians)
    {
        float sinv = sin(radians);
        float cosv = cos(radians);
        float xcopy = x;
        float ycopy = y;
        x = xcopy * cosv - ycopy * sinv;
        y = xcopy * sinv + ycopy * cosv;
    }

    // Rotation is in radians
    static DrawCmd& sprite_batch(      Texture&          texture,
                                 const Rectf&            srcrect,
                                 const Rectf&            dstrect,
                                 const float             rotation = 0.f,
                                 const ColorRGBAf&       color    = { 1.f, 1.f, 1.f, 1.f },
                                 const int               layer    = 0,
                                 const Renderer2DOrigin& origin   = OriginCenter,
                                 const bool              flipuvx  = false,
                                 const bool              flipuvy  = false,
                                 Shader&                 shader   = defaultShader)
    {
        ASSERT(inited); // Forgot to call Renderer2D::init()
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
        { std::swap(normalX, normalWidth); }
        if (flipuvy)
        { std::swap(normalY, normalHeight); }

        posAndCoords.emplace_back( dstrect.x , dstrect.y  , normalX,      normalY      );  // topleft
        posAndCoords.emplace_back( xpluswidth, dstrect.y  , normalWidth,  normalY      );  // topright
        posAndCoords.emplace_back( dstrect.x , yplusheight, normalX,      normalHeight );  // bottom left
        posAndCoords.emplace_back( xpluswidth, yplusheight, normalWidth,  normalHeight );  // bottom right
         
        if (rotation != 0)
        {
            Vector2f realOrigin;

            // If origin is default value, make it center of texture
            if (origin.pos.x == FLT_MAX)
            { realOrigin = getRectDefaultOrigin(dstrect); }
            else
            {
                if (origin.useWorldCoords)
                { realOrigin = origin.pos; }
                else
                { realOrigin = Vector2f(dstrect.x, dstrect.y) + origin.pos; }
            }

            for (size_t i = posAndCoords.size() - 4; i < posAndCoords.size(); i++)
            {
                auto& v = posAndCoords[i];
                v.x -= realOrigin.x; v.y -= realOrigin.y;
                rotate(v.x, v.y, rotation);
                v.x += realOrigin.x; v.y += realOrigin.y;
            }
        }

        return cmd;
    }

    static void prim_batch(const std::span<const Vector2f>&  points,
                           const ColorRGBAf&                 color = ColorRGBAf::white(),
                           const GLDrawMode                  mode  = GLDrawMode::LineStrip,
                           const int                         layer = DefaultPrimitiveLayer)
    {
        ASSERT(inited); // Forgot to call Renderer2D::init()
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

    static void text_batch(const String&     text,
                                 Font&       font,
                           const Vector2f&   pos,
                           const int         layer = DefaultTextLayer,
                           const ColorRGBAf& color = ColorRGBAf::white(),
                           const float       scale = 1.f)
    {
        ASSERT(inited); // Forgot to call Renderer2D::init()
        ASSERT(font.getAtlas().created());

        if (!font.created()) { return; }

        Vector2f currentPos = pos;
        WideString wide = toWide(text);
        for (auto& strchar : wide)
        {
            if (strchar == '\n')
            {
                currentPos.y += font.newLineHeight();
                currentPos.x = pos.x;
                continue;
            }

            FontAtlasChar* ch;
            if (!font.containsChar(strchar))
            { ch = &font.getFallbackChar(); }
            else
            { ch = &font.getChar(strchar); }

            float xpos = currentPos.x + (ch->bearing.x * scale);
            float ypos = currentPos.y - (ch->bearing.y + font.newLineHeight()) * scale;
            //float ypos = currentPos.y - (ch->bearing.y - ch->bearing.y) * scale;
            float w    = ch->rect.width  * scale;
            float h    = ch->rect.height * scale;
            currentPos.x += (ch->advance >> 6) * scale;

            sprite_batch(font.getAtlas(),
                         Rectf(ch->rect),
                         { xpos, ypos, w, h },
                         0.0f,
                         color,
                         layer,
                         OriginCenter,
                         false,
                         false,
                         textShader);
        }
    }

    static void onWindowResized()
    {
        //GLint viewport[4];
        //glGetIntegerv(GL_VIEWPORT, viewport);
        //auto fbSize = Renderer::getFramebufferSize();
        //// glViewport origin is bottom left, so i make it top left :))
        //glViewport(0, fbSize.y - viewport[3], viewport[2], viewport[3]);
    }
};
