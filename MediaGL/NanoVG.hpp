
// NanoVG isn't worth.
// Stop messing with my state!!!!

#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include "GLHelpers.hpp"
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
        void create(Renderer& renderer)
        {
            this->renderer = &renderer;
            ctx = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            tex.create();
            auto rect = renderer.getVirtualViewportRect();
            targetSize.x = rect.width;
            targetSize.y = rect.height;
            tex.setData(0, rect.width, rect.height, TextureFiltering::Linear, TexPixelFormats::RGBA, TexInternalFormats::RGBA, false);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex.glHandle, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        ~NanoVG()
        {
            if (!created()) { return; }
            nvgDeleteGL3(ctx);
            glDeleteFramebuffers(1, &fbo);
        }

        bool created() { return ctx != nullptr; }

        NVGcontext* ctx = nullptr;
        Renderer* renderer = nullptr;
        GLuint fbo = 0;
        Texture tex;
        Vector2f targetSize;

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

        void begin()
        {
            glState.reset();
            glDisable(GL_SCISSOR_TEST);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glStencilMask(0xffffffff);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            glStencilFunc(GL_ALWAYS, 0, 0xffffffff);

            auto rect = renderer->getVirtualViewportRect();
            auto winSize = renderer->window->getSize();
            int fbWidth, fbHeight;

            Vector2f tempTargetSize(rect.width, rect.height);
            if (tempTargetSize != targetSize)
            {
                tex.setData(0, rect.width, rect.height, TextureFiltering::Linear, TexPixelFormats::RGBA, TexInternalFormats::RGBA, false);
                targetSize = tempTargetSize;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glViewport(0, 0, rect.width, rect.height);

            renderer->clearColor({ 0.f, 0.f, 0.f, 0.f });
            SDL_GL_GetDrawableSize(renderer->window->window, &fbWidth, &fbHeight);
            nvgBeginFrame(ctx, rect.width, rect.height, (float)fbWidth / winSize.x);
        }

        inline void end()
        {
            nvgEndFrame(ctx);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            renderer->resetViewport();
            renderer->rescissor();
            glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
            renderer->drawTexture(tex, { 0, 0, targetSize.x, targetSize.y }, 0.f, { 1,1,1,1 });
            glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
        }

        operator NVGcontext* () { return ctx; }
    };
}
