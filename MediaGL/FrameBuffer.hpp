#pragma once

#include "GLHelpers.hpp"
#include "Texture.hpp"
#include "../DataStructures.hpp"
#include "Misc.hpp"

struct FrameBuffer
{
    GLuint glHandle = 0;
    Texture* texture = nullptr;

    void setTexture(Texture& texture)
    {
        this->texture = &texture;
        bind();
        texture.bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.glHandle, 0);
        unbind();
    }

    bool created()
    { return glHandle != 0; }

    void create()
    {
        GL_CHECK(glGenFramebuffers(1, &glHandle));
    }

    void reset()
    { if (created()) glDeleteFramebuffers(1, &glHandle); }

    void bind()
    { glBindFramebuffer(GL_FRAMEBUFFER, glHandle); }

    static void unbind()
    { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    FrameBuffer()  { }
    ~FrameBuffer() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};