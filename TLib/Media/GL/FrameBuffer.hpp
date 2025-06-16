#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Containers/FixedVector.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Types/Types.hpp>
#include <TLib/Misc.hpp>
#include <TLib/Macros.hpp>

enum class FrameBufferAttachmentType : GLenum
{
    Color0,
    Color1,
    Color2,
    Color3,
    Color4,
    Color5,
    Color6,
    Color7,
    Depth,
    Stencil,
    DepthStencil,
    Size
};

GLenum FBOAttachmentTypeToGLType(FrameBufferAttachmentType value)
{
    static UnorderedMap<FrameBufferAttachmentType, GLenum> map =
    {
        { FrameBufferAttachmentType::Color0,       GL_COLOR_ATTACHMENT0        },
        { FrameBufferAttachmentType::Color1,       GL_COLOR_ATTACHMENT1        },
        { FrameBufferAttachmentType::Color2,       GL_COLOR_ATTACHMENT2        },
        { FrameBufferAttachmentType::Color3,       GL_COLOR_ATTACHMENT3        },
        { FrameBufferAttachmentType::Color4,       GL_COLOR_ATTACHMENT4        },
        { FrameBufferAttachmentType::Color5,       GL_COLOR_ATTACHMENT5        },
        { FrameBufferAttachmentType::Color6,       GL_COLOR_ATTACHMENT6        },
        { FrameBufferAttachmentType::Color7,       GL_COLOR_ATTACHMENT7        },
        { FrameBufferAttachmentType::Depth,        GL_DEPTH_ATTACHMENT         },
        { FrameBufferAttachmentType::Stencil,      GL_STENCIL_ATTACHMENT       },
        { FrameBufferAttachmentType::DepthStencil, GL_DEPTH_STENCIL_ATTACHMENT }
    };

    return map[value];
}

struct FrameBuffer
{
private:
    GLuint glHandle = 0;
    Array<Texture*, (GLenum)FrameBufferAttachmentType::Size> textures;

    void move(FrameBuffer&& other)
    {
        reset();
        this->glHandle = other.glHandle;
        this->textures = other.textures;
        other.glHandle = 0;
    }

public:

    DISABLE_COPY(FrameBuffer);

    FrameBuffer()
    {
        textures.fill(nullptr);
    }

   ~FrameBuffer() { reset(); }

    FrameBuffer           (FrameBuffer&& other) noexcept { move(std::move(other)); }
    FrameBuffer& operator=(FrameBuffer&& other) noexcept { move(std::move(other)); }

    void setTexture(Texture& texture, FrameBufferAttachmentType type = FrameBufferAttachmentType::Color0)
    {
        if (!created()) { create(); }
        bind();
        texture.bind();
        GLenum index = (GLenum)type;
        textures[index] = &texture;
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, FBOAttachmentTypeToGLType(type), GL_TEXTURE_2D, texture.handle(), 0));
        unbind();
    }

    GLuint handle() const
    { return glHandle; }

    Texture* texture(FrameBufferAttachmentType attachment)
    {
        return textures[(size_t)attachment];
    }

    bool created() const
    { return glHandle != 0; }

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

        bool hasColorAttachment = false;
        {
            GLenum start = (GLenum)FrameBufferAttachmentType::Color0;
            GLenum stop  = (GLenum)FrameBufferAttachmentType::Color7;
            for (GLenum i = start; i <= stop; i++)
            {
                if (textures[i] != nullptr)
                { hasColorAttachment = true; break; }
            }
        }

        if (hasColorAttachment)
        { GL_CHECK(glDrawBuffer(GL_BACK)); GL_CHECK(glReadBuffer(GL_BACK)); }
        else // If there is NO color attatchments, set glRead/Draw buffer to GL_NONE
        { GL_CHECK(glDrawBuffer(GL_NONE));  GL_CHECK(glReadBuffer(GL_NONE)); }

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, glHandle));

        FixedVector<GLenum, (size_t)FrameBufferAttachmentType::Size> attachments;
        for (GLenum i = 0; i < textures.size(); i++)
        {
            if (textures[i])
            { attachments.push_back(FBOAttachmentTypeToGLType((FrameBufferAttachmentType)i)); }
        }
        GL_CHECK(glDrawBuffers(attachments.size(), attachments.data()));
    }

    static void unbind()
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        GL_CHECK(glDrawBuffer(GL_BACK)); GL_CHECK(glReadBuffer(GL_BACK));
    }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return  glHandle; }
};