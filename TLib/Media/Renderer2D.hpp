
#pragma once

#include <TLib/Types/Types.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Containers/Array.hpp>
#include <TLib/Containers/Pair.hpp>
#include <TLib/Containers/FixedVector.hpp>
#include <TLib/Containers/Span.hpp>
#include <TLib/Containers/Stack.hpp>
#include <TLib/Containers/Variant.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/Resource/Font.hpp>
#include <TLib/Media/RenderTarget.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Embed/Embed.hpp>
#include <TLib/Geometry.hpp>
#include <glm/gtx/rotate_vector.hpp>

using TexturePointers = FixedVector<Texture*, 4, false>;

struct Renderer2DRenderParams
{
    GLBlendMode srcBlendFactor = GLBlendMode::SrcAlpha;
    GLBlendMode dstBlendFactor = GLBlendMode::OneMinusSrcAlpha;

    // If true, will render using the default camera. Useful for drawing UI and FBOs
    bool ignoreView = false;
};



struct Renderer2DOrigin
{
    Renderer2DOrigin() = default;
    Renderer2DOrigin(const Vector2f& pos, bool useWorldCoords = false) : pos{pos}, useWorldCoords{useWorldCoords} { }

    // Uses local coords if this is false
    bool useWorldCoords = false;

    Vector2f pos = { FLT_MAX, FLT_MAX };
};

struct DrawTextureParams
{
    // Required
    TexturePointers  textures;
    Rectf            srcRect;
    Rectf            dstRect;

    // Optional
    float            rotation = 0; // Radians
    ColorRGBAf       color    = ColorRGBAf::white();
    Shader*          shader   = nullptr;
    Renderer2DOrigin origin;
    bool             flipUVx  = false;
    bool             flipUVy  = false;
    Vector2f         uvOffset;

    DrawTextureParams() = default;

    // Draw SubTexture to destination
    DrawTextureParams(SubTexture& subTexture, const Rectf& dstRect)
    {
        this->textures = { subTexture.texture };
        this->srcRect  = subTexture.rect;
        this->dstRect  = dstRect;
    }

    // Draw SubTexture with pos/rot/scale
    DrawTextureParams(SubTexture& texture, const Vector2f& position, float rotation = 0.f, const Vector2f& scale = Vector2f(1, 1))
    {
        Vector2f texSize     = Vector2f(texture.rect.getSize()) * scale;
        Vector2f halfTexSize = texSize / 2.f;
        Vector2f centerPos   = position - halfTexSize;

        this->textures = { texture.texture };
        this->srcRect  = texture.rect;
        this->dstRect  = Rectf(centerPos, texSize);
        this->rotation = rotation;
    }

    // Draw full Texture with pos/rot/scale
    DrawTextureParams(Texture& texture, const Vector2f& position, float rotation = 0.f, const Vector2f& scale = Vector2f(1, 1))
    {
        Vector2f texSize     = Vector2f(texture.getSize()) * scale;
        Vector2f halfTexSize = texSize / 2.f;
        Vector2f centerPos   = position - halfTexSize;

        this->textures = { &texture };
        this->srcRect  = Rectf(Vector2f(), Vector2f(texture.getSize()));
        this->dstRect  = Rectf(centerPos, texSize);
        this->rotation = rotation;
    }

    // Draw full Texture to destination
    DrawTextureParams(Texture& texture, const Rectf& dstRect)
    {
        this->textures = { &texture };
        this->srcRect  = Rectf(Vector2f(), Vector2f(texture.getSize()));
        this->dstRect  = dstRect;
    }
};

struct DrawQuadParams
{
    // Usual order for quad points are:
    // bottom left
    // bottom right
    // topleft
    // topright

    TexturePointers          textures;
    FixedVector<Vector2f, 4> UVs;
    FixedVector<Vector2f, 4> points;

    ColorRGBAf       color    = ColorRGBAf::white();
    Shader*          shader   = nullptr;
};

struct Renderer2D
{
#pragma region Public
public:
    static constexpr Vector2f OriginCenter = { FLT_MAX, FLT_MAX };

    static bool created()      { return inited; }
    static void create()       { init();        }

    /*
    @param ignoreCamera If true, will render using the default camera. Useful for drawing UI and FBOs
    */
    static void render(const Renderer2DRenderParams& params = Renderer2DRenderParams())
    {
        if (params.ignoreView)
        {
            auto oldView = currentView;
            Renderer2D::resetView();
            flush(params);
            currentView = oldView;
        }
        else
        {
            flush(params);
        }
    }

    static void setView(const View& view)
    {
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
    {
        auto& cmd = currentDrawCmds().commands.emplace_back().emplace<ClearCmd>();
        cmd.color = color;
    }

    static void drawTexture(const DrawTextureParams& params)
    {
        sprite_batch(params);
    }

    static void drawThickLines(const Span<Vector2f>& points,
                               float                 thickness = 1.f,
                               const ColorRGBAf&     color     = ColorRGBAf::white())
    {
        if (points.size() < 2) { return; }

        static Vector<Vector2f> finalPoints;
        finalPoints.clear();

        const float halfThickness = thickness / 2.f;
        Vector2f&   lastPoint     = points[0];
        for (size_t i = 1; i < points.size(); i++)
        {
            const Vector2f& currentPoint = points[i];

            float    angle        = lastPoint.angleTo(currentPoint);
            Vector2f upVector     = Vector2f(0.f, 1.f).rotated(angle);
            Vector2f displacement = upVector * halfThickness;

            finalPoints.push_back(lastPoint    + displacement);
            finalPoints.push_back(lastPoint    - displacement);
            finalPoints.push_back(currentPoint + displacement);
            finalPoints.push_back(currentPoint - displacement);

            lastPoint = currentPoint;
        }
        drawLines(finalPoints, color, GLDrawMode::TriangleStrip);
    }

    static void drawBezierCurve(const Span<Vector2f, 3>& points,
                                float                    thickness    = 1.f,
                                const ColorRGBAf&        color        = ColorRGBAf::white(),
                                const uint32_t           segmentCount = 16)
    {
        static Vector<Vector2f> finalPoints;
        finalPoints.clear();

        ASSERT(points.size() == 3);

        const Vector2f& P0 = points[0];
        const Vector2f& P1 = points[1];
        const Vector2f& P2 = points[2];
        float dt = 1.f / (segmentCount-1);
        for (uint32_t i = 0; i < segmentCount; i++)
        {
            float t = dt * i;
            float x = (1.f-t)*(1.f-t) * P0.x + 2.f * (1.f - t) * t * P1.x + t*t * P2.x;
            float y = (1.f-t)*(1.f-t) * P0.y + 2.f * (1.f - t) * t * P1.y + t*t * P2.y;
            finalPoints.push_back(Vector2f(x, y));
        }
        drawThickLines(finalPoints, thickness, color);
    }

    /*
    Draws an image using 9(slice/grid/patch) scaling
    https://en.wikipedia.org/wiki/9-slice_scaling

    If your (dstRect.width < left + right || dstRect.height < top + bottom)
    your image will probably look weird
    */
    [[deprecated]] // lmao this is useless
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

        //drawTexture(tex, mtSrc, mtDst); // Draw sides
        //drawTexture(tex, mlSrc, mlDst);
        //drawTexture(tex, mrSrc, mrDst);
        //drawTexture(tex, mbSrc, mbDst);
        //drawTexture(tex, tlSrc, tlDst); // Then corners
        //drawTexture(tex, trSrc, trDst);
        //drawTexture(tex, blSrc, blDst);
        //drawTexture(tex, brSrc, brDst);
        //drawTexture(tex, centerSrc, centerDst); // Then center
    }

    static void drawNinePatchTex(
            Texture& tex,
        const Rectf& dstRect,
        float left, float right, float top, float bottom)
    {
        drawNinePatchTex(tex, Rectf(0.f, 0.f, Vector2f(tex.getSize())), dstRect, left, right, top, bottom);
    }

    static void drawLines(const Span<const Vector2f>& points,
                          const ColorRGBAf&           color    = ColorRGBAf::white(),
                          GLDrawMode                  drawMode = GLDrawMode::LineStrip)
    {
        prim_batch(points, color, drawMode);
    }

    static void drawLine(const Vector2f&   start,
                         const Vector2f&   end,
                         const ColorRGBAf& color = ColorRGBAf::white())
    {
        Vector2f line[2] = { start, end };
        prim_batch(line, color, GLDrawMode::LineStrip);
    }

    static void drawRect(float                   x,
                         float                   y,
                         float                   w,
                         float                   h,
                         float                   rot    = 0.f,
                         bool                    filled = false,
                         const ColorRGBAf&       color  = ColorRGBAf::white(),
                         const Renderer2DOrigin& origin = OriginCenter)
    {
        Vector2f verts[4] = {
            Vector2f(x,     y    ),
            Vector2f(x + w, y    ),
            Vector2f(x + w, y + h),
            Vector2f(x,     y + h)
        };

        if (rot != 0)
        {
            Vector2f realOrigin;

            // If origin is default value, make it center of texture
            if (origin.pos.x == FLT_MAX)
            { realOrigin = getRectDefaultOrigin(Rectf(x, y, w, h)); }
            else
            {
                if (origin.useWorldCoords)
                { realOrigin = origin.pos; }
                else
                { realOrigin = Vector2f(x, y) + origin.pos; }
            }

            for (auto& v : verts)
            {
                v -= realOrigin;
                v.rotate(rot);
                v += realOrigin;
            }
        }

        prim_batch(verts, color, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop);
    }

    static void drawRect(const Rectf&            rect,
                         float                   rot    = 0.f,
                         bool                    filled = false,
                         const ColorRGBAf&       color  = ColorRGBAf::white(),
                         const Renderer2DOrigin& origin = OriginCenter)
    {
        drawRect(rect.x, rect.y, rect.width, rect.height, rot, filled, color, origin);
    }

    static void drawGrid(const Vector2f&   offset,
                         const Vector2i&   gridCount,
                         const Vector2f&   gridSize,
                         const ColorRGBAf& color     = ColorRGBAf::white())
    {
        const float targetX = gridCount.x * gridSize.x;
        const float targetY = gridCount.y * gridSize.y;

        for (int x = 0; x <= gridCount.x; x++)
        {
            const float worldX = x * gridSize.x;
            drawLine(
                Vector2f{ worldX, 0       } + offset,
                Vector2f{ worldX, targetY } + offset,
                color);
        }

        for (int y = 0; y <= gridCount.y; y++)
        {
            const float worldY = y * gridSize.y;
            drawLine(
                Vector2f{ 0,       worldY } + offset,
                Vector2f{ targetX, worldY } + offset,
                color);
        }
    }

    static void drawCircle(const Vector2f&   pos,
                           const float       rad,
                           const bool        filled       = false,
                           const ColorRGBAf& color        = ColorRGBAf::white(),
                           const int         segmentCount = 16)
    {
        const float theta            = glm::pi<float>() * 2.f / static_cast<float>(segmentCount);
        const float tangetial_factor = tanf(theta);
        const float radial_factor    = cosf(theta);

        float x = rad;
        float y = 0;

        static Vector<Vector2f> points;
        points.clear();

        for (int i = 0; i < segmentCount; i++)
        {
            points.push_back(Vector2f{ x + pos.x, y + pos.y });

            float tx = -y;
            float ty =  x;
            x += tx * tangetial_factor;
            y += ty * tangetial_factor;
            x *= radial_factor;
            y *= radial_factor;
        }

        GLDrawMode mode = filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop;
        prim_batch(points, color, mode);
    }

    // Uses radians for arc and rotation.
    static void drawSemiCircle(const Geom::SemiCircleParams& params,
                               const ColorRGBAf&             color     = ColorRGBAf::white(),
                               bool                          filled    = false,
                               float                         thickness = 0.f)
    {
        Geom::Geometry2D points = Geom::semiCircle(params);

        if (filled || thickness == 0.f)
        {
            GLDrawMode mode = filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop;
            drawLines(points, color, mode);
        }
        else
        {
            drawThickLines(points, thickness, color);
        }

    }

    static void drawTriangle(const Vector2f&   pos,  // Center of triangle
                             const Vector2f&   size, // Width and height
                             float             rot = 0.f,
                             bool              filled = false,
                             const ColorRGBAf& color = ColorRGBAf::white())
    {
        const Vector2f halfSize = size / 2.f;
        Array<Vector2f, 3> points;
        points[0].y -= halfSize.y; // top
        points[1].x += halfSize.x; // bottom right
        points[1].y += halfSize.y;
        points[2].x -= halfSize.x; // bottom left
        points[2].y += halfSize.y;
        if (rot != 0.f)
        {
            for (auto& p : points) { p.rotate(rot); }
        }
        points[0] += pos;
        points[1] += pos;
        points[2] += pos;
        Renderer2D::drawLines(points, color, filled ? GLDrawMode::TriangleFan : GLDrawMode::LineLoop);
    }

    static void drawText(const String&     text,
                         Font&             font,
                         const Vector2f&   pos,
                         const ColorRGBAf& color = ColorRGBAf::white(),
                         const float       scale = 1.f)
    {
        text_batch(text, font, pos, color, scale);
    }

    static void drawChar(wchar_t                 ch,
                         Font&                   font,
                         const Rectf&            dstrect,
                         const float             rotation = 0.f,
                         const ColorRGBAf&       color    = ColorRGBAf::white(),
                         const Renderer2DOrigin& origin   = OriginCenter,
                         const bool              flipuvx  = false,
                         const bool              flipuvy  = false,
                         Shader&                 shader   = textShader)
    {
        DrawTextureParams p;
        p.textures = {&font.getAtlas()};
        p.srcRect  = font.getCharTex(ch).rect;
        p.dstRect  = dstrect;
        p.rotation = rotation;
        p.color    = color;
        p.origin   = origin;
        p.flipUVx  = flipuvx;
        p.flipUVy  = flipuvy;
        p.shader   = &shader;

        sprite_batch(p);
    }

    static void drawChar(wchar_t             ch,
                     Font&                   font,
                     const Vector2f&         pos,
                     const float             rotation = 0.f,
                     const ColorRGBAf&       color    = ColorRGBAf::white(),
                     const Renderer2DOrigin& origin   = OriginCenter,
                     const bool              flipuvx  = false,
                     const bool              flipuvy  = false,
                     Shader&                 shader   = textShader)
    {
        Rectf srcRect = font.getCharTex(ch).rect;
        Rectf dstRect = Rectf(pos - (srcRect.getSize()/2.f), srcRect.getSize());

        drawChar(ch, font, dstRect, rotation,
             color, origin, flipuvx, flipuvy, shader);
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
    static inline    Vector<uint32_t> sprite_indices = { 0, 2, 1, 1, 2, 3 };
    static inline    Texture          whiteTex;
    static inline    GLubyte          whiteTexData[1][1][4] = { { {255, 255, 255, 255} } };
    static constexpr GLuint           restartIndex = std::numeric_limits<GLuint>::max();
    static inline    GPUVertexData    mesh;
    static inline    Shader           defaultShader;
    static inline    Shader           textShader;
    static inline    View             currentView;
    static inline    bool             inited = false;
    static inline    float            sdfTextWidth;
    static inline    float            sdfTextEdge;

    struct ISpan
    {
        uint32_t index = 0;
        uint32_t size  = 0;

        uint32_t begin() const { return index; }
        uint32_t end()   const { return index + size; }
    };

    #pragma pack(push, 0)
    struct VerticesDrawCmdVert
    {
        glm::vec2 vert;
        glm::vec4 color;
        glm::vec2 uv;
    };
    #pragma pack(pop)

    struct VerticesDrawCmd
    {
        GLDrawMode                  drawMode;
        TexturePointers             textures = { &whiteTex };
        Shader*                     shader   = &defaultShader;
        Vector<VerticesDrawCmdVert> vertices;
        Vector<uint32_t>            indices;
    };

    struct VerticesMaskedDrawCmd : VerticesDrawCmd
    {
        Texture* mask = nullptr;
        ISpan    maskUVs;
    };

    struct ClearCmd
    {
        ColorRGBAf color;
    };

    using DrawCmd = Variant<VerticesDrawCmd, ClearCmd>;

public:

    struct DrawCommands
    {
        Vector<DrawCmd> commands;

        DrawCommands()
        {
            size_t reserveSize = 512;
            commands.reserve(reserveSize);
        }

        void clear()
        {
            commands.clear();
        }
    };

    static inline void drawCommandsPush(DrawCommands& yourDrawCmds)
    {
        drawCmdStack.push(&yourDrawCmds);
    }

    static inline void drawCommandsPop()
    {
        ASSERT(drawCmdStack.size() > 1);
        drawCmdStack.pop();
    }

private:
    // TODO: actually use the frustum
    // need to calculate AABB of the sprites rect to check collision
    // checking for primitives isn't necessary
    static inline Frustum frustum;

    // Draw data goes here
    static inline DrawCommands         drawCmds;
    static inline Stack<DrawCommands*> drawCmdStack = { &drawCmds };
    static inline DrawCommands& currentDrawCmds() { return *drawCmdStack.top(); }

    // These buffers are copied to the GPU
    static inline Vector<VerticesDrawCmdVert> batchBuffer;
    static inline Vector<uint32_t>            batchBufferIndices;

    #pragma region Util

    static inline glm::vec4 vec4(const ColorRGBAf& color)
    { return glm::vec4(color.r, color.g, color.b, color.a); }

    static inline void rotate(float& x, float& y, float radians)
    {
        float sinv = sin(-radians);
        float cosv = cos(-radians);
        float xcopy = x;
        float ycopy = y;
        x = xcopy * cosv - ycopy * sinv;
        y = xcopy * sinv + ycopy * cosv;
    }

    static inline float interpolate(float from, float to, float percent)
    {
        float difference = to - from;
        return from + ( difference * percent );
    }

    #pragma endregion

    struct FlushState
    {
        TexturePointers textures = {};
        Shader*         shader   = nullptr;
        GLDrawMode      drawMode = GLDrawMode::None;

        bool null() const { return shader == nullptr; }

        bool operator==(const FlushState& fs) const
        {
            return textures == fs.textures &&
                   shader   == fs.shader   &&
                   drawMode == fs.drawMode;
        }

        bool operator!=(const FlushState& fs) const { return !(*this == fs); }
    };

    static void init()
    {
        if (inited) { return; }

        if (!Renderer::created())
        { Renderer::create(); }

        if (!mesh.valid())
        { mesh.setLayout({ TLib::Layout::Vec2f(), TLib::Layout::Vec4f(), TLib::Layout::Vec2f() }); }

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

        size_t reserveSize = size_t(1024)*2;
        batchBuffer         .reserve(reserveSize);
        batchBufferIndices  .reserve(reserveSize);
        inited = true;
    }

    static void flushCurrent(FlushState& fs, const Renderer2DRenderParams& params)
    {
        if (batchBuffer.empty() || fs.null()) { return; }

        for (int32_t i = 0; i < fs.textures.size(); i++)
        { fs.textures[i]->bind(i); }

        fs.shader->bind();
        fs.shader->setMat4f("projection", currentView.getMatrix());

        mesh.setData   (batchBuffer,        AccessType::Dynamic);
        mesh.setIndices(batchBufferIndices, AccessType::Dynamic);

        RenderState rs;
        rs.drawMode       = fs.drawMode;
        rs.srcBlendFactor = params.srcBlendFactor;
        rs.dstBlendFactor = params.dstBlendFactor;
        Renderer::draw(*fs.shader, mesh, rs);

        batchBuffer.clear();
        batchBufferIndices.clear();
    }

    static void flush(const Renderer2DRenderParams& params)
    {
        auto& cmds = currentDrawCmds();

        if (cmds.commands.empty()) { return; }

        // Projection uniform for shader is set in flushCurrent()
        // TODO: Frustum is unused for now
        //auto mat = camera.getMatrix();
        //frustum  = Frustum(mat);

        Vector2f fbSize(Renderer::getFramebufferSize());
        if (RenderTarget::getBoundRenderTarget())
        { fbSize = Vector2f(RenderTarget::getBoundRenderTarget()->getSize()); }
        Renderer::setViewport(getViewportSizePixels(currentView, fbSize));

        // Multisample causes texture bleeding.
        // They still happen, but are less frequent with multisample disabled
        // To fix it completely, center your texels
        glDisable(GL_MULTISAMPLE); 
        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(restartIndex);

        FlushState newState;
        FlushState state;

        auto flushIfNewState = [&](const FlushState& newState)
        {
            bool stateChangeImminent = newState != state;
            if (stateChangeImminent)
            {
                flushCurrent(state, params);
                state = newState;
            }
        };

        for (auto& cmd : cmds.commands)
        {
            if (cmd.is<VerticesDrawCmd>())
            {
                auto& vdCmd = cmd.as<VerticesDrawCmd>();

                newState.textures = vdCmd.textures;
                newState.shader   = vdCmd.shader;
                newState.drawMode = vdCmd.drawMode;
                flushIfNewState(newState);

                size_t nextIndex = batchBuffer.size();
                for (auto& i : vdCmd.indices)
                { batchBufferIndices.push_back(nextIndex + i); }
                batchBufferIndices.push_back(restartIndex);
                batchBuffer.push_back(vdCmd.vertices);
            }
            else if (cmd.is<ClearCmd>())
            {
                flushCurrent(state, params);
                Renderer::clearColor(cmd.as<ClearCmd>().color);
            }
        }

        flushCurrent(newState, params);
        cmds.clear();
    }

    static VerticesDrawCmd& sprite_batch(const DrawTextureParams& params)
    {
        ASSERT(inited); // Forgot to call Renderer2D::init()

        auto& cmds = currentDrawCmds();
        VerticesDrawCmd& cmd = cmds.commands.emplace_back().emplace<VerticesDrawCmd>();

        if (params.shader == nullptr) { cmd.shader = &defaultShader; }
        else                          { cmd.shader = params.shader;  }

        cmd.textures = params.textures;
        cmd.drawMode = GLDrawMode::Triangles;

        cmd.indices.push_back(sprite_indices);

        const Pair<Vector2f, Vector2f> uv = getTextureUVs(*params.textures[0], params.srcRect);
        auto uv_x      = uv.first.x  + params.uvOffset.x;
        auto uv_y      = uv.first.y  + params.uvOffset.y;
        auto uv_width  = uv.second.x + params.uvOffset.x;
        auto uv_height = uv.second.y + params.uvOffset.y;

        if (params.flipUVx) { std::swap(uv_x, uv_width); }
        if (params.flipUVy) { std::swap(uv_y, uv_height); }

        float xpluswidth  = params.dstRect.x + params.dstRect.width;
        float yplusheight = params.dstRect.y + params.dstRect.height;

        Array<VerticesDrawCmdVert, 4> vertices =
        {
            VerticesDrawCmdVert{ glm::vec2( params.dstRect.x, params.dstRect.y ), vec4(params.color), glm::vec2( uv_x,     uv_y      ) }, // bottom left
            VerticesDrawCmdVert{ glm::vec2(       xpluswidth, params.dstRect.y ), vec4(params.color), glm::vec2( uv_width, uv_y      ) }, // bottom right
            VerticesDrawCmdVert{ glm::vec2( params.dstRect.x, yplusheight      ), vec4(params.color), glm::vec2( uv_x,     uv_height ) }, // topleft
            VerticesDrawCmdVert{ glm::vec2(       xpluswidth, yplusheight      ), vec4(params.color), glm::vec2( uv_width, uv_height ) }  // topright
        };

        if (params.rotation != 0)
        {
            Vector2f realOrigin;

            // If origin is default value, make it center of texture
            if (params.origin.pos.x == FLT_MAX)
            { realOrigin = getRectDefaultOrigin(params.dstRect); }
            else
            {
                if (params.origin.useWorldCoords)
                { realOrigin = params.origin.pos; }
                else
                { realOrigin = Vector2f(params.dstRect.x, params.dstRect.y) + params.origin.pos; }
            }

            for (auto& vertex : vertices)
            {
                vertex.vert.x -= realOrigin.x; vertex.vert.y -= realOrigin.y;
                rotate(vertex.vert.x, vertex.vert.y, params.rotation);
                vertex.vert.x += realOrigin.x; vertex.vert.y += realOrigin.y;
            }
        }

        cmd.vertices.push_back(vertices);

        return cmd;
    }

public:
    static VerticesDrawCmd& quad_batch(const DrawQuadParams& params)
    {
        ASSERT(inited); // Forgot to call Renderer2D::init()

        auto& cmds = currentDrawCmds();
        VerticesDrawCmd& cmd = cmds.commands.emplace_back().emplace<VerticesDrawCmd>();

        if (params.shader == nullptr) { cmd.shader = &defaultShader; }
        else                          { cmd.shader = params.shader; }

        cmd.textures = params.textures;
        cmd.drawMode = GLDrawMode::Triangles;

        cmd.indices.push_back(sprite_indices);

        Array<VerticesDrawCmdVert, 4> vertices =
        {
            VerticesDrawCmdVert{ glm::vec2( params.points[0].x, params.points[0].y ), vec4(params.color), glm::vec2( params.UVs[0].x, params.UVs[0].y ) }, // bottom left
            VerticesDrawCmdVert{ glm::vec2( params.points[1].x, params.points[1].y ), vec4(params.color), glm::vec2( params.UVs[1].x, params.UVs[1].y ) }, // bottom right
            VerticesDrawCmdVert{ glm::vec2( params.points[2].x, params.points[2].y ), vec4(params.color), glm::vec2( params.UVs[2].x, params.UVs[2].y ) }, // topleft
            VerticesDrawCmdVert{ glm::vec2( params.points[3].x, params.points[3].y ), vec4(params.color), glm::vec2( params.UVs[3].x, params.UVs[3].y ) }  // topright
        };

        cmd.vertices.push_back(vertices);

        return cmd;
    }

private:
    static void prim_batch(const Span<const Vector2f>& points,
                           const ColorRGBAf&           color = ColorRGBAf::white(),
                           const GLDrawMode            mode  = GLDrawMode::LineStrip)
    {
        ASSERT(inited); // Forgot to call Renderer2D::init()
        if (points.empty()) { return; }

        auto& cmds = currentDrawCmds();
        VerticesDrawCmd& cmd = cmds.commands.emplace_back().emplace<VerticesDrawCmd>();
        
        cmd.textures = { &whiteTex };
        cmd.drawMode = mode;

        for (auto& point : points)
        { cmd.vertices.emplace_back(glm::vec2(point.x, point.y), vec4(color), glm::vec2(0.f, 0.f)); }

        for (size_t i = 0; i < points.size(); i++)
        { cmd.indices.push_back(i); }
    }

    static void text_batch(const String&     text,
                                 Font&       font,
                           const Vector2f&   pos,
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
                currentPos.y += font.lineSpacing();
                currentPos.x = pos.x;
                continue;
            }

            FontAtlasChar* ch;
            if (!font.containsChar(strchar))
            { ch = &font.getFallbackChar(); }
            else
            { ch = &font.getChar(strchar); }

            const float originYOffset = ch->rect.height - ch->bearing.y;

            float xpos = currentPos.x + ch->bearing.x * scale;
            float ypos = currentPos.y - originYOffset * scale;
            float w    = ch->rect.width  * scale;
            float h    = ch->rect.height * scale;
            currentPos.x += (ch->advance >> 6) * scale;

            DrawTextureParams p;
            p.textures = { &font.getAtlas() };
            p.srcRect  = Rectf(ch->rect);
            p.dstRect  = { xpos, ypos, w, h };
            p.rotation = 0.f;
            p.color    = color;
            p.shader   = &textShader;

            sprite_batch(p);
        }
    }

#pragma endregion
};
