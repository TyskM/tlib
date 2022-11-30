#pragma once

#include "GLHelpers.hpp"
#include "GLState.hpp"
#include "Misc.hpp"

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

    // Pass NoCreate to delay construction
    VertexArray() { this->create(); }
    VertexArray(NoCreateT) { }
    ~VertexArray() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};

#undef glGenVertexArrays;
#undef glDeleteBuffers;
#undef glBindVertexArray;
#define glGenVertexArrays(x) static_assert(false, "Disabled in VertexArray.hpp")
#define glDeleteBuffers(x)   static_assert(false, "Disabled in VertexArray.hpp")
#define glBindVertexArray(x) static_assert(false, "Disabled in VertexArray.hpp")
