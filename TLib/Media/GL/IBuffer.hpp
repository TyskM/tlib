//
// Created by Ty on 2023-01-30.
//

#pragma once

#include <TLib/Media/Logging.hpp>
#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/GL/GLState.hpp>
#include <TLib/NonAssignable.hpp>
#include <vector>

enum class AccessType : GLenum
{
    Static  [[maybe_unused]] = GL_STATIC_DRAW,
    Dynamic [[maybe_unused]] = GL_DYNAMIC_DRAW,
    Stream  [[maybe_unused]] = GL_STREAM_DRAW
};

// This class is not intended to be used directly.
// See: VertexBuffer, ElementBuffer
struct IBuffer : NonCopyable
{
    GLuint glHandle = 0;

    bool created() const { return glHandle != 0; }

    void create()
    {
        reset();
        GL_CHECK(glGenBuffers(1, &glHandle));
        rendlog->info("Created buffer at location {}", glHandle);
    }

    void reset()
    {
        if (created())
        {
            rendlog->info("Destroyed buffer at location {}", glHandle);
            glDeleteBuffers(1, &glHandle);
            glHandle = 0;
        }
    }

    IBuffer() = default;
    ~IBuffer() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }

    IBuffer(IBuffer&& src) noexcept
    {
        glHandle = src.glHandle;
        src.glHandle = 0;
    }

    IBuffer& operator=(IBuffer&& src) noexcept
    {
        reset();
        glHandle = src.glHandle;
        src.glHandle = 0;
        return *this;
    }
};