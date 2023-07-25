#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/Resource/Texture.hpp>
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
    { glBindFramebuffer(GL_FRAMEBUFFER, glHandle); }

    static void unbind()
    { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

    FrameBuffer()  { }
    ~FrameBuffer() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};