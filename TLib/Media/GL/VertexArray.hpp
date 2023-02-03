#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/GL/GLState.hpp>
#include <TLib/Media/Logging.hpp>

// Used for storing information about a VertexBuffer
struct VertexArray
{
    GLuint glHandle = 0;

    bool created() const { return glHandle != 0; }

    void create()
    {
        GL_CHECK(glGenVertexArrays(1, &glHandle));
        rendlog->info("Created VAO at location {}", glHandle);
    }

    void reset()
    {
        if (created())
        {
            rendlog->info("Destroyed VAO at location {}", glHandle);
            glDeleteBuffers(1, &glHandle);
            glHandle = 0;
        }
    }

    void bind()
    {
        ASSERT(created());
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
    ~VertexArray() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint()  { return glHandle; }
};
