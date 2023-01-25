#pragma once

#include "GLHelpers.hpp"
#include "GLState.hpp"
#include "Misc.hpp"

// Used for storing information about a VertexBuffer
struct VertexArray
{
    GLuint glHandle = 0;

    bool created() { return glHandle != 0; }

    void create()
    {
        GL_CHECK(glGenVertexArrays(1, &glHandle));
    }

    void reset()
    {
        if (created()) glDeleteBuffers(1, &glHandle);
    }

    void bind()
    {
        if (glState.boundVertexArray == this) { return; }
        GL_CHECK(glBindVertexArray(glHandle));
        glState.boundVertexArray = this;
    }

    static void unbind()
    {
        GL_CHECK(glBindVertexArray(0));
        glState.boundVertexArray = nullptr;
    }

    VertexArray() = default;
    VertexArray(NoCreateT) { }
    ~VertexArray() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};
