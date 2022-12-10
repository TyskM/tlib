#pragma once

#include "GLHelpers.hpp"
#include "GLState.hpp"
#include "Misc.hpp"

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

    // Pass NoCreate to delay construction
    GLBuffer() { this->create(); }
    GLBuffer(NoCreateT) { }
    ~GLBuffer() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};

struct VertexBuffer : GLBuffer
{
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

struct ElementBuffer : GLBuffer
{
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