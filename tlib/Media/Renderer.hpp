#pragma once

// A lot of the primitive drawing code was taken from SDL2_gfx : https://github.com/keera-studios/SDL2_gfx

#include <string>
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_gfxPrimitives_font.h>
#include "../NonAssignable.hpp"
#include "../DataStructures.hpp"
#include "../Macros.hpp"
#include "Font.hpp"
#include <vector>
#undef main

int _rendererCompareInt(const void *a, const void *b)
{
    return (*(const int *) a) - (*(const int *) b);
}

struct Renderer : NonAssignable
{
    // Internal vars
    inline static constexpr int _subsys = SDL_INIT_VIDEO;
    SDL_Renderer* _renderer = nullptr;
    bool _created = false;
    ColorRGBAi _clearColor = { 45, 45, 45 };
    int* gfxPrimitivesPolyInts = NULL;
    int  gfxPrimitivesPolyAllocated = 0;

    void create(SDL_Window* window, bool useLinearFiltering = false) noexcept
    {
        SDL_Init(_subsys);
        
        if( !( IMG_Init(IMG_INIT_PNG)) )
        { printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError()); abort(); }

        // TODO: enable direct3d AA
        // https://blog.karatos.in/a?ID=01050-2840056b-fff5-45eb-9217-64f34a19b7e6
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, useLinearFiltering); // Enable AA (works only with opengl)
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // AA samples
        
        _renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        if (_renderer == nullptr)
        { SDL_Log("Could not create a renderer: %s", SDL_GetError()); abort(); }

        SDL_RendererInfo rendererInfo;
        SDL_GetRendererInfo(_renderer, &rendererInfo);

        std::cout << "using renderer backend: " << rendererInfo.name << std::endl;

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

    SDL_Texture* loadTexture(const std::string& path) noexcept
    {
        SDL_Texture* newTexture = NULL;

        SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
        if( loadedSurface == NULL )
        { printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() ); }
        else
        {
            newTexture = SDL_CreateTextureFromSurface( _renderer, loadedSurface );
            if( newTexture == NULL )
            { printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() ); }

            SDL_FreeSurface( loadedSurface );
        }

        return newTexture;
    }

    inline void setClearColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
    {
        _clearColor = {r, g, b, a};
    }

    inline void setClearColor(ColorRGBAi color) noexcept
    {
        _clearColor = color;
        setClearColor(color.r, color.g, color.b, color.a);
    }

    inline void clear() noexcept
    {
        SDL_RenderClear(_renderer);
    }

    inline void present() noexcept
    {
        setRenderColor(_clearColor);
        SDL_RenderPresent(_renderer);
    }

    inline void drawTextureSection(SDL_Texture* texture, const SDL_Rect& targetRect, const SDL_Rect& srcRect) noexcept
    {
        SDL_RenderCopy(_renderer, texture, &srcRect, &targetRect);
    }

    inline void drawTexture(SDL_Texture* texture, const SDL_Rect& targetRect) noexcept
    {
        SDL_RenderCopy(_renderer, texture, NULL, &targetRect);
    }

    inline void drawTexture(SDL_Texture* texture, const SDL_Rect& rect, double rot)
    {
        SDL_RenderCopyEx(_renderer, texture, NULL, &rect, rot, NULL, SDL_FLIP_NONE);
    }

    inline void drawTexture(SDL_Texture* texture, Vector2i pos, double rot)
    {
        const auto s = getTextureSize(texture);
        SDL_Rect r = { pos.x, pos.y, s.x, s.y };
        SDL_RenderCopyEx(_renderer, texture, NULL, &r, rot, NULL, SDL_FLIP_NONE);
    }

    inline void drawTexture(SDL_Texture* texture, Vector2i pos)
    {
        const auto s = getTextureSize(texture);
        SDL_Rect r = { pos.x, pos.y, s.x, s.y };
        SDL_RenderCopy(_renderer, texture, NULL, &r);
    }

    inline Font createFont(const std::string& fontpath, ColorRGBAi color = { 0, 0, 0, 255 }, int ptsize = 30, FontStyle style = FontStyle::NORMAL) noexcept
    {
        return Font(*this, fontpath, color, ptsize, style);
    }

    // FC_Draw formatting is bugged? Use std::format instead.
    inline void drawText(FC_Font* font, Vector2i pos, const std::string& text) noexcept
    {
        FC_Draw(font, _renderer, pos.x, pos.y, text.c_str());
    }

    inline void setRenderColor(const ColorRGBAi& color) noexcept { SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a); }
    inline void setRenderColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept { SDL_SetRenderDrawColor(_renderer, r, g, b, a); }

    const ColorRGBAi getRenderColor() const noexcept
    {
        Uint8 r; Uint8 g; Uint8 b; Uint8 a;
        SDL_GetRenderDrawColor(_renderer, &r, &g, &b, &a);
        return ColorRGBAi{ r, g, b, a };
    }

    template <typename PointType>
    inline void drawLine(PointType p1, PointType p2, ColorRGBAi color) noexcept
    {
        setRenderColor(color);
        drawLine(p1, p2);
    }

    template <typename PointType>
    inline void drawLine(PointType p1, PointType p2) noexcept
    {
        ASSERT(SDL_RenderDrawLine(_renderer, p1.x, p1.y, p2.x, p2.y) == 0);
    }

    template<typename Container>
    inline void drawLines(const Container& vertices) noexcept
    { drawLines(vertices.data(), vertices.size()); }

    template<typename PointType>
    inline void drawLines(const PointType* verticesArray, size_t verticesCount) noexcept
    {
        if (verticesCount < 2) { return; }

        for (size_t i = 1; i < verticesCount; i++)
        {
            size_t prev = i - 1;
            drawLine(verticesArray[prev], verticesArray[i]);
        }
    }

    template<typename Container>
    inline void drawLines(const Container& vertices, const ColorRGBAi& color) noexcept
    {
        setRenderColor(color);
        drawLines(vertices);
    }

    template<typename PointType>
    inline void drawLines(const PointType* verticesArray, size_t verticesCount, const ColorRGBAi& color) noexcept
    {
        setRenderColor(color);
        drawLines(verticesArray, verticesCount);
    }

    template<typename Container>
    inline void drawPolygon(const Container& vertices) noexcept
    { drawPolygon(vertices.data(), vertices.size()); }

    template<typename PointType>
    inline void drawPolygon(const PointType* verticesArray, size_t verticesCount) noexcept
    {
        ASSERT(verticesCount >= 3);

        drawLines(verticesArray, verticesCount);
        drawLine(verticesArray[0], verticesArray[verticesCount - 1]);
    }

    template<typename Container>
    inline void drawPolygon(const Container& vertices, const ColorRGBAi& color) noexcept
    {
        setRenderColor(color);
        drawPolygon(vertices);
    }

    template<typename PointType>
    inline void drawPolygon(const PointType* verticesArray, size_t verticesCount, const ColorRGBAi& color) noexcept
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
                SDL_RenderDrawLine(_renderer, xa, y, xb, y);
            }
        }
    }

    template<typename Container>
    inline void drawFilledPolygon(const Container& points, ColorRGBAi color) noexcept
    { drawFilledPolygon(points.data(), points.size(), color); }

    inline void drawRect(int x, int y, int w, int h, ColorRGBAi color)
    {
        rectangleRGBA(_renderer, x, y, (x + w), (y + h), color.r, color.g, color.b, color.a);
    }

    inline void drawRectFilled(int x, int y, int w, int h, ColorRGBAi color)
    {
        boxRGBA(_renderer, x, y, (x + w), (y + h), color.r, color.g, color.b, color.a);
    }

    inline void drawRoundedRect(int x, int y, int w, int h, int rounding, ColorRGBAi color)
    {
        roundedRectangleRGBA(_renderer, x, y, (x + w), (y + h), rounding, color.r, color.g, color.b, color.a);
    }

    inline void drawRoundedRectFilled(int x, int y, int w, int h, int rounding, ColorRGBAi color)
    {
        roundedBoxRGBA(_renderer, x, y, (x + w), (y + h), rounding, color.r, color.g, color.b, color.a);
    }

    inline void drawEllipse(int x, int y, int rx, int ry, ColorRGBAi color)
    {
        ellipseRGBA(_renderer, x, y, rx, ry, color.r, color.g, color.b, color.a);
    }

    inline void drawEllipseFilled(int x, int y, int rx, int ry, ColorRGBAi color)
    {
        filledEllipseRGBA(_renderer, x, y, rx, ry, color.r, color.g, color.b, color.a);
    }

    inline void drawCircle(int x, int y, int rad, ColorRGBAi color)
    {
        circleRGBA(_renderer, x, y, rad, color.r, color.g, color.b, color.a);
    }

    inline void drawCircleFilled(int x, int y, int rad, ColorRGBAi color)
    {
        filledCircleRGBA(_renderer, x, y, rad, color.r, color.g, color.b, color.a);
    }

    inline void drawTriangle(int x1,int y1, int x2, int y2, int x3, int y3, ColorRGBAi color)
    {
        trigonRGBA(_renderer, x1,y1, x2,y2, x3,y3, color.r, color.g, color.b, color.a);
    }

    inline void drawTriangleFilled(int x1,int y1, int x2, int y2, int x3, int y3, ColorRGBAi color)
    {
        filledTrigonRGBA(_renderer, x1,y1, x2,y2, x3,y3, color.r, color.g, color.b, color.a);
    }

    inline void drawArc(int x, int y, int rad, int start, int end, ColorRGBAi color)
    {
        arcRGBA(_renderer, x,y, rad, start,end, color.r, color.g, color.b, color.a);
    }

    operator SDL_Renderer*() const { return _renderer; }
    operator SDL_Renderer&() const { return *_renderer; }

};

