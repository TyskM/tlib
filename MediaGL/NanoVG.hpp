
#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <gl/gl3w.h>
#define NVG_NO_STB
#include "../thirdparty/nanovg/nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "../thirdparty/nanovg/nanovg_gl.h"
#include "../NonAssignable.hpp"
#include "../DataStructures.hpp"
#include "Renderer.hpp"
#include <iostream>

namespace vg
{
    using Font = int;

    struct NanoVG : NonAssignable
    {
        NVGcontext* ctx;

        Font loadFont(const char* path, const char* fontName)
        {
            auto f = nvgCreateFont(ctx, fontName, path);
            if (f == -1)
            { std::cerr << "Could not add font: " << fontName << std::endl; }
            return f;

        }

        inline void setFont(Font font)
        {
            nvgFontFaceId(ctx, font);
        }

        inline void drawText(const Vector2f& pos, const std::string& text, const char* end = NULL)
        {
            nvgText(ctx, pos.x, pos.y, text.c_str(), end);
        }

        // Remember to reset the global glState object before
        // using tlib renderer again
        inline void begin(const Renderer& r)
        {
            auto winSize = r.window->getSize();
            nvgBeginFrame(ctx, winSize.x, winSize.y, r._view.bounds.width / winSize.x);
        }

        inline void end()
        { nvgEndFrame(ctx); }

        NanoVG()
        {
            ctx = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
        }

        ~NanoVG()
        {
            nvgDeleteGL3(ctx);
        }

        operator NVGcontext* () { return ctx; }
    };
}
