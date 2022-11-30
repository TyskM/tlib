#pragma once

#include "GLHelpers.hpp"
#include "GLState.hpp"
#include "Misc.hpp"

struct VertexBuffer
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

    // Pass NoCreate to delay construction
    VertexBuffer() { this->create(); }
    VertexBuffer(NoCreateT) { }
    ~VertexBuffer() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};
