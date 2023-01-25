#pragma once

#include "GLHelpers.hpp"
#include "GLState.hpp"
#include "Misc.hpp"

enum class AccessType : int
{
    Static  = GL_STATIC_DRAW,
    Dynamic = GL_DYNAMIC_DRAW,
    Stream  = GL_STREAM_DRAW
};

// This class is not intended to be used directly.
// See: VertexBuffer, ElementBuffer
struct GLBuffer
{
    GLuint glHandle = 0;

    bool created() { return glHandle != 0; }

    void create()
    {
        GL_CHECK(glGenBuffers(1, &glHandle));
    }

    void reset()
    {
        if (created()) glDeleteBuffers(1, &glHandle);
    }

    GLBuffer() = default;
    GLBuffer(NoCreateT) { }
    ~GLBuffer() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};

// Used for storing indices
// You probably also want a VertexArray
struct VertexBuffer : GLBuffer
{
    template <typename T>
    void bufferData(const std::vector<T>& data, AccessType accessType)
    {
        bind();
        glBufferData(GL_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), static_cast<int>(accessType));
    }

    void bind()
    {
        if (glState.boundVertexBuffer == this) { return; }
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, glHandle));
        glState.boundVertexBuffer = this;
    }

    static void unbind()
    {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        glState.boundVertexBuffer = nullptr;
    }
};

// Also known as an index buffer
// For storing indices
struct ElementBuffer : GLBuffer
{
    template <typename T>
    void bufferData(const std::vector<T>& data, AccessType accessType)
    {
        static_assert(std::is_same<T, int>::value, "Using non ints for an indice buffer is prolly a mistake?");
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), static_cast<int>(accessType));
    }

    void bind()
    {
        if (glState.boundElementBuffer == this) { return; }
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glHandle));
        glState.boundElementBuffer = this;
    }

    static void unbind()
    {
        GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        glState.boundElementBuffer = nullptr;
    }
};