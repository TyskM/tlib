#pragma once

#include "GLHelpers.hpp"
#include "Texture.hpp"
#include "View.hpp"
#include "../DataStructures.hpp"

struct FrameBuffer
{
    GLuint glHandle = 0;

    FrameBuffer() { }
    ~FrameBuffer()
    {
        if (glHandle != 0) { glDeleteFramebuffers(1, &glHandle); }
    }

    void create()
    {
        glGenFramebuffers(1, &glHandle);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, glHandle);
    }

    static inline void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

struct RenderBuffer
{
    GLuint glHandle = 0;

    RenderBuffer() = default;
    ~RenderBuffer()
    {
        if (glHandle != 0) { glDeleteRenderbuffers(1, &glHandle); }
    }

    void create()
    {
        glGenRenderbuffers(1, &glHandle);
    }

    void bind()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, glHandle);
    }

    static inline void unbind()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
};

struct RenderTarget
{
    FrameBuffer fb;
    Texture tex;
    View view;

    RenderTarget() { }

    RenderTarget(Vector2i size, TextureFiltering texFilter = TextureFiltering::Linear)
    {
        create(size, texFilter);
    }

    void create(Vector2i size, TextureFiltering texFilter = TextureFiltering::Linear)
    {
        fb.create();
        tex.create(size, texFilter);
    }
};