#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/GL/GLState.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Misc.hpp>

struct FrameBuffer
{
private:
    GLuint glHandle = 0;
    Texture* texturePtr = nullptr;

public:
    void setTexture(Texture& texture)
    {
        if (!created()) { create(); }
        this->texturePtr = &texture;
        bind();
        texture.bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.handle(), 0);
        unbind();
    }

    GLuint handle() const
    { return glHandle; }

    Texture* texture()
    { return texturePtr; }

    bool created() const
    { return glHandle != 0; }

    bool valid() const
    { return created() && texturePtr != nullptr; }

    void create()
    { GL_CHECK(glGenFramebuffers(1, &glHandle)); }

    void reset()
    { if (created()) glDeleteFramebuffers(1, &glHandle); }

    void bind()
    {
        ASSERT(created()); // Create me, idiot.
        if (glState.boundFrameBuffer == this) { return; }
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, glHandle));
        //GL_CHECK(glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE));
        glState.boundFrameBuffer = this;
    }

    static void unbind()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        //GL_CHECK(glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE));
        glState.boundFrameBuffer = nullptr;
    }

    FrameBuffer()  { }
    ~FrameBuffer() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};