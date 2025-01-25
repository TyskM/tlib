#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/GL/GLState.hpp>
#include <TLib/Types/Types.hpp>
#include <TLib/Misc.hpp>
#include <TLib/Macros.hpp>

enum class FrameBufferAttachmentType : GLenum
{
    Depth        = GL_DEPTH_ATTACHMENT,
    Stencil      = GL_STENCIL_ATTACHMENT,
    DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
    Color0       = GL_COLOR_ATTACHMENT0,
    Color1       = GL_COLOR_ATTACHMENT1,
    Color2       = GL_COLOR_ATTACHMENT2,
    Color3       = GL_COLOR_ATTACHMENT3,
    Color4       = GL_COLOR_ATTACHMENT4,
    Color5       = GL_COLOR_ATTACHMENT5,
    Color6       = GL_COLOR_ATTACHMENT6,
    Color7       = GL_COLOR_ATTACHMENT7,
    Null         = GL_NONE
};

struct FrameBuffer
{
private:
    GLuint   glHandle   = 0;
    Texture* texturePtr = nullptr;

    // TODO: Support multiple attatchments
    FrameBufferAttachmentType currentType = FrameBufferAttachmentType::Null;

public:

    DISABLE_COPY(FrameBuffer);
    DISABLE_MOVE(FrameBuffer);
    FrameBuffer() = default;
   ~FrameBuffer() { reset(); }

    // TODO: auto reset binding to prev state, for now this unbinds if bound.
    void setTexture(Texture& texture, FrameBufferAttachmentType type = FrameBufferAttachmentType::Color0)
    {
        if (!created()) { create(); }
        bind();
        texture.bind();
        texturePtr  = &texture;
        currentType = type;
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(type), GL_TEXTURE_2D, texture.handle(), 0));
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
    {
        reset();
        GL_CHECK(glGenFramebuffers(1, &glHandle));
    }

    void reset()
    { if (created()) glDeleteFramebuffers(1, &glHandle); }

    void bind()
    {
        ASSERT(created()); // Create me, idiot.

        GLenum ct = static_cast<GLenum>(currentType);
        if (ct >= GL_COLOR_ATTACHMENT0 && ct <= GL_COLOR_ATTACHMENT31)
        { GL_CHECK(glDrawBuffer(GL_BACK)); GL_CHECK(glReadBuffer(GL_BACK)); }
        else // If there is NO color attatchments, set glRead/Draw buffer to GL_NONE
        { GL_CHECK(glDrawBuffer(GL_NONE));  GL_CHECK(glReadBuffer(GL_NONE)); }

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, glHandle));
    }

    static void unbind()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        GL_CHECK(glDrawBuffer(GL_BACK)); GL_CHECK(glReadBuffer(GL_BACK));
    }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};