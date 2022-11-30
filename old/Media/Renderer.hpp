#pragma once

#include <string>
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_gfxPrimitives_font.h>
#include <string>
#include "Font.hpp"
#include "../Macros.hpp"
#include "../NonAssignable.hpp"
#include "../DataStructures.hpp"
#include "../Event.hpp"

// A lot of the primitive drawing code was taken from SDL2_gfx : https://github.com/keera-studios/SDL2_gfx
// TODO: Convert primitive draw functions to use SDL_RenderDrawLinesF

#undef main

static int _rendererEventWatch(void* userdata, SDL_Event* event);

static int _rendererCompareInt(const void* a, const void* b)
{
    return (*(const int *) a) - (*(const int *) b);
}

struct ScaledRenderTargetData
{
    SDL_Rect rect;
    float scale;
};

struct Renderer : NonAssignable
{
    struct Texture : NonCopyable
    {
        SDL_Texture* _texture = nullptr;

        Texture() = default;

        Texture(Texture&& t) noexcept
        {
            destroy();
            _texture = t._texture;
            t._texture = nullptr;
        }

        Texture& operator=(Texture&& t) noexcept
        {
            destroy();
            _texture = t._texture;
            t._texture = nullptr;
            return *this;
        }

        Texture(SDL_Texture* tex) { destroy(); _texture = tex; }

        Texture(Renderer& renderer, const std::string& path)
        { loadFromPath(renderer, path); }

        // Manual creation
        Texture(Renderer& renderer, SDL_PixelFormatEnum format, SDL_TextureAccess access, int w, int h)
        { create(renderer, format, access, w, h); }

        // Manual creation
        // Uses renderer.getInfo().texture_formats[0] for format
        Texture(Renderer& renderer, SDL_TextureAccess access, int w, int h)
        { create(renderer, access, w, h); }

        ~Texture()
        { destroy(); }

        bool isCreated() const { return _texture != nullptr; }

        inline void loadFromPath(Renderer& renderer, const std::string& path) noexcept
        {
            destroy();
            const auto texPtr = renderer.loadTexture(path);
            if (texPtr != nullptr)
                _texture = texPtr;
        }

        inline void create(Renderer& renderer, SDL_PixelFormatEnum format, SDL_TextureAccess access, int w, int h)
        {
            destroy();
            _texture = SDL_CreateTexture(renderer, format, access, w, h);
        }

        // Uses renderer.getInfo().texture_formats[0] for format
        inline void create(Renderer& renderer, SDL_TextureAccess access, int w, int h)
        {
            const auto info = renderer.getInfo();
            create(renderer, static_cast<SDL_PixelFormatEnum>(info.texture_formats[0]), access, w, h);
        }

        void create(Renderer& renderer, SDL_Surface* surface)
        {
            _texture = SDL_CreateTextureFromSurface(renderer._renderer, surface);
        }

        Vector2i getSize() const
        {
            Vector2i rsize;
            SDL_QueryTexture(_texture, NULL, NULL, &rsize.x, &rsize.y);
            return rsize;
        }

        uint32_t getFormat() const
        {
            uint32_t rformat;
            SDL_QueryTexture(_texture, &rformat, NULL, NULL, NULL);
            return rformat;
        }

        void destroy()
        {
            if (_texture != nullptr) { SDL_DestroyTexture(_texture); }
            _texture = nullptr;
        }

        operator SDL_Texture*() { return _texture; }
    };

    /// Internal vars
     static constexpr int _subsys = SDL_INIT_VIDEO;
    SDL_Renderer* _renderer = nullptr;
    bool _created = false;
    int* _gfxPrimitivesPolyInts = NULL;
    int  _gfxPrimitivesPolyAllocated = 0;
    Vector2f _dpiScale; // Read only

    // 1: New window size
    Event<void(Vector2i)> event_resized;

    struct Camera
    {
        Vector2f pos;
        float rot = 0.f; // In degrees
        float zoom = 1.f; // 1.0 is default
        bool centered = false;
    };
    Camera _backupCamera; // used for restoring camera when enableCamera() and disableCamera() are called
    Camera camera;

    // Render scale quality accepts values 0-2. Use 0 for pixel art.
    void create(SDL_Window* window, bool useLinearFiltering = false, int renderScaleQuality = 2) noexcept
    {
        SDL_Init(_subsys);

        if( !( IMG_Init(IMG_INIT_PNG)) )
        { printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError()); abort(); }

        SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING, "1");
        // TODO: enable direct3d AA
        // https://blog.karatos.in/a?ID=01050-2840056b-fff5-45eb-9217-64f34a19b7e6
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, std::to_string(renderScaleQuality).c_str());
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1); // TODO: remove if not needed for video player
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, useLinearFiltering); // Enable AA (works only with opengl)
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // AA samples
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

        _renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE );

        if (_renderer == nullptr)
        { SDL_Log("Could not create a renderer: %s", SDL_GetError()); abort(); }

        SDL_RendererInfo rendererInfo;
        SDL_GetRendererInfo(_renderer, &rendererInfo);

        std::cout << "using renderer backend: " << rendererInfo.name << std::endl;

        updateDPIScale();
        SDL_AddEventWatch(&_rendererEventWatch, this);

        _created = true;
    }

    ~Renderer()
    {
        if (_created)
        {
            SDL_DestroyRenderer(_renderer);
            IMG_Quit();
            SDL_QuitSubSystem(_subsys);
        }
    }

    Vector2i getTextureSize(SDL_Texture* tex) const
    {
        Vector2i rsize;
        SDL_QueryTexture(tex, NULL, NULL, &rsize.x, &rsize.y);
        return rsize;
    }

    Vector2i getOutputSize() const noexcept
    {
        Vector2i s;
        SDL_GetRendererOutputSize(_renderer, &s.x, &s.y);
        return s;
    }

    SDL_RendererInfo getInfo()
    {
        SDL_RendererInfo info;
        SDL_GetRendererInfo(_renderer, &info);
        return info;
    }

    SDL_Window* getWindow() const noexcept
    { return SDL_RenderGetWindow(_renderer); }

    void setRenderColor(const ColorRGBAi& color) noexcept
    { setRenderColor(color.r, color.g, color.b, color.a); }

    void setRenderColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
    { SDL_SetRenderDrawColor(_renderer, r, g, b, a); }

    const ColorRGBAi getRenderColor() const noexcept
    {
        Uint8 r; Uint8 g; Uint8 b; Uint8 a;
        SDL_GetRenderDrawColor(_renderer, &r, &g, &b, &a);
        return ColorRGBAi{ r, g, b, a };
    }

    void updateDPIScale()
    {
        int output_w, output_h, window_w, window_h;
        SDL_GetRendererOutputSize(_renderer, &output_w, &output_h);
        SDL_GetWindowSize(SDL_RenderGetWindow(_renderer), &window_w, &window_h);
        _dpiScale = { static_cast<float>(window_w) / output_w, static_cast<float>(window_h) / output_h };
    }

    void setTarget(SDL_Texture* tex)
    { SDL_SetRenderTarget(_renderer, tex); }

    // Sets the render target back to the window
    void resetTarget()
    { SDL_SetRenderTarget(_renderer, NULL); }

    void clear(ColorRGBAi color) noexcept
    {
        setRenderColor(color);
        SDL_RenderClear(_renderer);
    }

    void present() noexcept
    {
        SDL_RenderPresent(_renderer);
    }

    #pragma region Camera
    // TODO: Function for getting mouse in world space
    void setCamera(Vector2f position, float zoom = 1.f, float rotation = 0)
    {
        ASSERTWARN(zoom > 0, "setCamera zoom param is less than zero. Clamp your zoom!!!");
        camera = { position, rotation, zoom };
    }

    void resetCamera()
    { camera = Camera(); }

    void disableCamera()
    { _backupCamera = camera; resetCamera(); camera.centered = false; }

    void enableCamera()
    { camera = _backupCamera; }

    template <typename PointType>
    Vector2f toCamCoords(PointType v)
    {
        if (camera.centered)
            return Vector2f{ v.x - camera.pos.x, v.y - camera.pos.y } + Vector2f(getOutputSize()) / 2;
        else
            return Vector2f{ v.x - camera.pos.x, v.y - camera.pos.y };
    }

    Vector2f toCamCoords(float x, float y)
    { return toCamCoords(Vector2f{ x, y }); }

    Vector2f getMouseWorldPos()
    {
        int x, y;
        SDL_GetMouseState(&x, &y);
        return -toCamCoords(-x, -y);
    }

    Vector2i getMouseWorldGridPos(Vector2f gridSize)
    {
        return posToGridPos(getMouseWorldPos(), gridSize);
    }

    Vector2i posToGridPos(Vector2f pos, Vector2f gridSize)
    {
        // Integer division truncates towards 0
        // Floor manually so it doesn't break with negative values
        return Vector2i((pos / gridSize).floored());
    }
    #pragma endregion

    #pragma region Textures
    SDL_Texture* loadTexture(const std::string& path) noexcept
    {
        SDL_Texture* newTexture = NULL;

        SDL_Surface* loadedSurface = IMG_Load(path.c_str());
        if (loadedSurface == NULL)
        { printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError()); return nullptr; }
        else
        {
            newTexture = SDL_CreateTextureFromSurface(_renderer, loadedSurface);
            if (newTexture == NULL)
            { printf("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError()); return nullptr; }

            SDL_FreeSurface(loadedSurface);
        }

        return newTexture;
    }

    void drawTexture(Texture& texture, const SDL_Rect* srcrect,
                     const SDL_Rect* dstrect, const double angle = 0.f,
                     const SDL_Point* center = NULL,
                     const SDL_RendererFlip flip = SDL_FLIP_NONE) noexcept
    {
        if (!texture.isCreated()) { return; }
        if (dstrect)
        {
            const auto camCoords = toCamCoords(dstrect->x, dstrect->y);
            SDL_Rect camDst = { camCoords.x, camCoords.y, dstrect->w, dstrect->h };
            SDL_RenderCopyEx(_renderer, texture, srcrect, &camDst, angle, center, flip);
        }
        else
        { SDL_RenderCopyEx(_renderer, texture, srcrect, dstrect, angle, center, flip); }
    }

    void drawTextureSection(Texture& texture, const SDL_Rect& srcRect, const SDL_Rect& dstRect,
                            const float angle = 0, const SDL_Point* center = NULL,
                            const SDL_RendererFlip flip = SDL_FLIP_NONE) noexcept
    {
        drawTexture(texture, &srcRect, &dstRect, angle, center, flip);
    }

    void drawTexture(Texture& texture, const SDL_Rect& dstRect,
                     const float angle = 0, const SDL_Point* center = NULL,
                     const SDL_RendererFlip flip = SDL_FLIP_NONE) noexcept
    {
        drawTexture(texture, NULL, &dstRect, angle, center, flip);
    }

    void drawTexture(Texture& texture, Vector2f pos,
       const float angle = 0, const SDL_Point* center = NULL, const SDL_RendererFlip flip = SDL_FLIP_NONE) noexcept
    {
        const auto s = getTextureSize(texture);
        SDL_Rect r = { pos.x - s.x/2, pos.y - s.y/2, s.x, s.y };
        drawTexture(texture, NULL, &r, angle, center, flip);
    }
    #pragma endregion

    #pragma region Font
    Font createFont(const std::string& fontpath, ColorRGBAi color = { 0, 0, 0, 255 },
                    int ptsize = 30, FontStyle style = FontStyle::NORMAL) noexcept
    {
        return Font(*this, fontpath, color, ptsize, style);
    }

    // FC_Draw formatting is bugged? Use std::format instead.
    void drawText(FC_Font* font, Vector2f pos, const std::string& text, Vector2f scale = {1,1}, bool centered = true, bool ignoreCamera = false) noexcept
    {
        if (centered)
        { pos.x -= static_cast<float>( (FC_GetWidth(font, text.c_str())) / 2) * scale.x; }

        if (ignoreCamera)
        { FC_DrawScale(font, _renderer, pos.x, pos.y, {scale.x, scale.y}, text.c_str()); }
        else
        {
            const auto newpos = toCamCoords(pos);
            FC_DrawScale(font, _renderer, newpos.x, newpos.y, {scale.x, scale.y}, text.c_str());
        }
    }
    #pragma endregion

    #pragma region Primitives
    template <typename PointType>
    void drawLine(PointType p1, PointType p2) noexcept
    {
        const auto np1 = toCamCoords(p1);
        const auto np2 = toCamCoords(p2);
        ASSERT(SDL_RenderDrawLineF(_renderer, np1.x, np1.y,
                                              np2.x, np2.y) == 0);
    }

    template <typename PointType>
    void drawLine(PointType p1, PointType p2, ColorRGBAi color) noexcept
    {
        setRenderColor(color);
        drawLine(p1, p2);
    }

    template<typename Container>
    void drawLines(const Container& vertices) noexcept
    { drawLines(vertices.data(), vertices.size()); }

    template<typename PointType>
    void drawLines(const PointType* verticesArray, size_t verticesCount) noexcept
    {
        if (verticesCount < 2) { return; }

        for (size_t i = 1; i < verticesCount; i++)
        {
            size_t prev = i - 1;
            drawLine(verticesArray[prev], verticesArray[i]);
        }
    }

    template<typename Container>
    void drawLines(const Container& vertices, const ColorRGBAi& color) noexcept
    {
        setRenderColor(color);
        drawLines(vertices);
    }

    template<typename PointType>
    void drawLines(const PointType* verticesArray, size_t verticesCount, const ColorRGBAi& color) noexcept
    {
        setRenderColor(color);
        drawLines(verticesArray, verticesCount);
    }

    template<typename Container>
    void drawPolygon(const Container& vertices) noexcept
    { drawPolygon(vertices.data(), vertices.size()); }

    template<typename PointType>
    void drawPolygon(const PointType* verticesArray, size_t verticesCount) noexcept
    {
        ASSERT(verticesCount >= 3);

        drawLines(verticesArray, verticesCount);
        drawLine(verticesArray[0], verticesArray[verticesCount - 1]);
    }

    template<typename Container>
    void drawPolygon(const Container& vertices, const ColorRGBAi& color) noexcept
    {
        setRenderColor(color);
        drawPolygon(vertices);
    }

    template<typename PointType>
    void drawPolygon(const PointType* verticesArray, size_t verticesCount, const ColorRGBAi& color) noexcept
    {
        setRenderColor(color);
        drawPolygon(verticesArray, verticesCount);
    }

    template<typename PointType>
    void drawFilledPolygon(PointType* points, size_t count, ColorRGBAi color) noexcept
    {
        int i;
        int y, xa, xb;
        int miny, maxy;
        int x1, y1;
        int x2, y2;
        int ind1, ind2;
        int ints;
        int *gfxPrimitivesPolyInts = NULL;
        int *gfxPrimitivesPolyIntsNew = NULL;
        int gfxPrimitivesPolyAllocated = 0;

        ASSERT(count >= 3);

        /*
        * Allocate temp array, only grow array 
        */
        if (!gfxPrimitivesPolyAllocated)
        {
            gfxPrimitivesPolyInts = (int *) malloc(sizeof(int) * count);
            gfxPrimitivesPolyAllocated = count;
        }
        else {
            if (gfxPrimitivesPolyAllocated < count) {
                gfxPrimitivesPolyIntsNew = (int *) realloc(gfxPrimitivesPolyInts, sizeof(int) * count);
                if (!gfxPrimitivesPolyIntsNew) {
                    if (!gfxPrimitivesPolyInts) {
                        free(gfxPrimitivesPolyInts);
                        gfxPrimitivesPolyInts = NULL;
                    }
                    gfxPrimitivesPolyAllocated = 0;
                } else {
                    gfxPrimitivesPolyInts = gfxPrimitivesPolyIntsNew;
                    gfxPrimitivesPolyAllocated = count;
                }
            }
        }

        if (gfxPrimitivesPolyInts==NULL) { gfxPrimitivesPolyAllocated = 0; }

        gfxPrimitivesPolyInts =  gfxPrimitivesPolyInts;
        gfxPrimitivesPolyAllocated = gfxPrimitivesPolyAllocated;


        ASSERT(gfxPrimitivesPolyInts != NULL);

        miny = points[0].y;
        maxy = points[0].y;
        for (i = 1; (i < count); i++)
        {
            if (points[i].y < miny)
            {
                miny = points[i].y;
            }
            else if (points[i].y > maxy)
            {
                maxy = points[i].y;
            }
        }

        SDL_SetRenderDrawBlendMode(_renderer, (color.a == 255) ? SDL_BLENDMODE_NONE : SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);

        for (y = miny; (y <= maxy); y++)
        {
            ints = 0;
            for (i = 0; (i < count); i++)
            {
                if (!i)
                {
                    ind1 = count - 1;
                    ind2 = 0;
                }
                else
                {
                    ind1 = i - 1;
                    ind2 = i;
                }
                y1 = points[ind1].y;
                y2 = points[ind2].y;
                if (y1 < y2)
                {
                    x1 = points[ind1].x;
                    x2 = points[ind2].x;
                }
                else if (y1 > y2)
                {
                    y2 = points[ind1].y;
                    y1 = points[ind2].y;
                    x2 = points[ind1].x;
                    x1 = points[ind2].x;
                }
                else { continue; }
                if ( ((y >= y1) && (y < y2)) || ((y == maxy) && (y > y1) && (y <= y2)) )
                {
                    gfxPrimitivesPolyInts[ints++] = ((65536 * (y - y1)) / (y2 - y1)) * (x2 - x1) + (65536 * x1);
                } 	    
            }

            qsort(gfxPrimitivesPolyInts, ints, sizeof(int), _rendererCompareInt);

            for (i = 0; (i < ints); i += 2)
            {
                xa = gfxPrimitivesPolyInts[i] + 1;
                xa = (xa >> 16) + ((xa & 32768) >> 15);
                xb = gfxPrimitivesPolyInts[i+1] - 1;
                xb = (xb >> 16) + ((xb & 32768) >> 15);
                drawLine(Vector2f(xa, y), Vector2f(xb, y));
            }
        }
    }

    template<typename Container>
     void drawFilledPolygon(const Container& points, ColorRGBAi color) noexcept
    { drawFilledPolygon(points.data(), points.size(), color); }

    void drawRect(int x, int y, int w, int h, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        rectangleRGBA(_renderer, npos.x, npos.y, (npos.x + w), (npos.y + h),
                      color.r, color.g, color.b, color.a);
    }

    void drawRectFilled(int x, int y, int w, int h, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        boxRGBA(_renderer, npos.x, npos.y, (npos.x + w), (npos.y + h),
                color.r, color.g, color.b, color.a);
    }

    void drawRoundedRect(int x, int y, int w, int h, int rounding, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        roundedRectangleRGBA(_renderer, npos.x, npos.y, (npos.x + w), (npos.y + h),
                             rounding, color.r, color.g, color.b, color.a);
    }

    void drawRoundedRectFilled(int x, int y, int w, int h, int rounding, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        roundedBoxRGBA(_renderer, npos.x, npos.y, (npos.x + w), (npos.y + h),
                       rounding, color.r, color.g, color.b, color.a);
    }

    void drawEllipse(int x, int y, int rx, int ry, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        ellipseRGBA(_renderer, npos.x, npos.y, rx, ry, color.r, color.g, color.b, color.a);
    }

    void drawEllipseFilled(int x, int y, int rx, int ry, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        filledEllipseRGBA(_renderer, npos.x, npos.y, rx, ry, color.r, color.g, color.b, color.a);
    }

    void drawCircle(int x, int y, int rad, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        circleRGBA(_renderer, npos.x, npos.y, rad, color.r, color.g, color.b, color.a);
    }

    void drawCircleFilled(int x, int y, int rad, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        filledCircleRGBA(_renderer, npos.x, npos.y, rad, color.r, color.g, color.b, color.a);
    }

    void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, ColorRGBAi color)
    {
        const auto npos1 = toCamCoords(x1, y1);
        const auto npos2 = toCamCoords(x2, y2);
        const auto npos3 = toCamCoords(x3, y3);
        trigonRGBA(_renderer, npos1.x,npos1.y, npos2.x,npos2.y, npos3.x,npos3.y, color.r, color.g, color.b, color.a);
    }

    void drawTriangleFilled(int x1, int y1, int x2, int y2, int x3, int y3, ColorRGBAi color)
    {
        const auto npos1 = toCamCoords(x1, y1);
        const auto npos2 = toCamCoords(x2, y2);
        const auto npos3 = toCamCoords(x3, y3);
        filledTrigonRGBA(_renderer, npos1.x,npos1.y, npos2.x,npos2.y, npos3.x,npos3.y, color.r, color.g, color.b, color.a);
    }

    void drawArc(int x, int y, int rad, int start, int end, ColorRGBAi color)
    {
        const auto npos = toCamCoords(x, y);
        arcRGBA(_renderer, npos.x,npos.y, rad, start,end, color.r, color.g, color.b, color.a);
    }
    #pragma endregion

    operator SDL_Renderer*() const { return _renderer; }
    operator SDL_Renderer&() const { return *_renderer; }
};

static int _rendererEventWatch(void* userdata, SDL_Event* event)
{
    Renderer* renderer = static_cast<Renderer*>(userdata);
    switch (event->type)
    {
    case SDL_WINDOWEVENT:
        if (event->window.event == SDL_WINDOWEVENT_MOVED)
        {
            renderer->updateDPIScale();
            
        }
        else if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        {
            renderer->updateDPIScale();
            renderer->event_resized(Vector2i{ event->window.data1, event->window.data2 });
        }
        break;
    default: break;
    }
    return 0;
}

using Texture = Renderer::Texture;

// TODO: make this actually work lmao
struct VirtualTarget
{
    Texture _virtualTex;
    Vector2f _targetVirtualRes;
    SDL_Rect _virtualRect;
    float _virtualScale;
    Renderer* _renderer;

    VirtualTarget() = default;
    VirtualTarget(Renderer& renderer)
    { create(renderer); }

    void create(Renderer& renderer)
    { _renderer = &renderer; }

    void setVirtualResolution(Vector2f res)
    {
        _targetVirtualRes = res;
        _virtualTex.create(*_renderer, SDL_TEXTUREACCESS_TARGET, res.x, res.y);
    }

    // Draws the virtual res texture to the current target
    // You probably want to call resetTarget() first
    void drawVirtualTarget()
    {
        _renderer->drawTexture(_virtualTex, NULL, &_virtualRect);
    }

    void updateVirtualRes()
    {
        const auto r = _getIdealTargetViewportRect(_targetVirtualRes);
        _virtualRect = r.rect;
        _virtualScale = r.scale;
    }

    // For internal use
    ScaledRenderTargetData _getIdealTargetViewportRect(Vector2f logicalSize) const
    {
        // Taken from:
        // https://github.com/davidsiaw/SDL2/blob/6ecaa6b61372e5b2f9bd01201814d07e34bb4186/src/render/SDL_render.c
        // UpdateLogicalSize

        int w = 1, h = 1;
        float want_aspect;
        float real_aspect;
        float scale;
        SDL_Rect viewport;
        int scale_policy = 0; /* 0 is for letterbox, 1 is for overscan */
        const char* hint;

        SDL_GetRendererOutputSize(*_renderer, &w, &h);

        hint = SDL_GetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE);
        if (hint && (*hint == '1' || SDL_strcasecmp(hint, "overscan") == 0))
        {
#if SDL_VIDEO_RENDER_D3D
            SDL_bool overscan_supported = SDL_TRUE;
            /* Unfortunately, Direct3D 9 doesn't support negative viewport numbers
            which the overscan implementation relies on. */
            if (SDL_strcasecmp(SDL_GetCurrentVideoDriver(), "direct3d") == 0) {
                overscan_supported = SDL_FALSE;
            }
            if (overscan_supported) {
                scale_policy = 1;
            }
#else
            scale_policy = 1;
#endif
        }

        want_aspect = logicalSize.x / logicalSize.y;
        real_aspect = (float)w / h;

        /* Clear the scale because we're setting viewport in output coordinates */
        //SDL_RenderSetScale(renderer, 1.0f, 1.0f);

        if (SDL_RenderGetIntegerScale(*_renderer))
        {
            if (want_aspect > real_aspect)
            { scale = (float)(w / logicalSize.x); }
            else { scale = (float)(h / logicalSize.y); }
            viewport.w = (int)SDL_ceil(logicalSize.x * scale);
            viewport.x = (w - viewport.w) / 2;
            viewport.h = (int)SDL_ceil(logicalSize.y * scale);
            viewport.y = (h - viewport.h) / 2;
            return { viewport, scale };
        }
        else if (SDL_fabs(want_aspect-real_aspect) < 0.0001)
        {
            /* The aspect ratios are the same, just scale appropriately */
            scale = (float)w / logicalSize.x;
            SDL_RenderSetViewport(*_renderer, NULL);
            SDL_Texture* target = SDL_GetRenderTarget(*_renderer);
            bool winIsTarget = target == NULL;
            int x, y;
            if (!winIsTarget)
            { SDL_QueryTexture(target, NULL, NULL, &x, &y); }
            else
            { SDL_GetWindowSize(SDL_RenderGetWindow(*_renderer), &x, &y); }
            return { {0, 0, x, y}, scale };
        }
        else if (want_aspect > real_aspect)
        {
            if (scale_policy == 1)
            {
                /* We want a wider aspect ratio than is available - 
                zoom so logical height matches the real height 
                and the width will grow off the screen */
                scale = (float)h / logicalSize.y;
                viewport.y = 0;
                viewport.h = h;
                viewport.w = (int)SDL_ceil(logicalSize.x * scale);
                viewport.x = (w - viewport.w) / 2;
                return { viewport, scale };
            }
            else
            {
                /* We want a wider aspect ratio than is available - letterbox it */
                scale = (float)w / logicalSize.x;
                viewport.x = 0;
                viewport.w = w;
                viewport.h = (int)SDL_ceil(logicalSize.y * scale);
                viewport.y = (h - viewport.h) / 2;
                return { viewport, scale };
            }
        }
        else
        {
            if (scale_policy == 1)
            {
                /* We want a narrower aspect ratio than is available -
                zoom so logical width matches the real width
                and the height will grow off the screen
                */
                scale = (float)w / logicalSize.x;
                viewport.x = 0;
                viewport.w = w;
                viewport.h = (int)SDL_ceil(logicalSize.y * scale);
                viewport.y = (h - viewport.h) / 2;
                return { viewport, scale };
            }
            else
            {
                /* We want a narrower aspect ratio than is available - use side-bars */
                scale = (float)h / logicalSize.y;
                viewport.y = 0;
                viewport.h = h;
                viewport.w = (int)SDL_ceil(logicalSize.x * scale);
                viewport.x = (w - viewport.w) / 2;
                return { viewport, scale };
            }
        }
    }

    void useVirtualResTarget()
    { _renderer->setTarget(_virtualTex); };

    Vector2f localToWorld(int winx, int winy)
    {
        float physx, physy;

        physx = ((float) winx) / _renderer->_dpiScale.x;
        physy = ((float) winy) / _renderer->_dpiScale.y;

        SDL_Rect r;
        SDL_RenderGetViewport(*_renderer, &r);

        float x = (float)((physx - _virtualRect.x/*r.x*/) / _virtualScale);
        float y = (float)((physy - _virtualRect.y/*r.y*/) / _virtualScale);
        return { x, y };
    }
};