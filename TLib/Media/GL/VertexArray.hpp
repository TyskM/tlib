#pragma once

#include <TLib/Macros.hpp>
#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Media/Logging.hpp>

// Used for storing information about a ArrayBuffer
struct VertexArray
{
private:
    GLuint glHandle = 0;

    void move(VertexArray& other)
    {
        reset();
        glHandle = other.glHandle;
        other.glHandle = 0;
    }

public:
    DISABLE_COPY(VertexArray);

    bool created() const { return glHandle != 0; }

    void create()
    {
        GL_CHECK(glGenVertexArrays(1, &glHandle));

        if constexpr(verboseRendererLogging)
            rendlog->info("Created VAO at location {}", glHandle);
    }

    void reset()
    {
        if (created())
        {
            if constexpr(verboseRendererLogging)
                rendlog->info("Destroyed VAO at location {}", glHandle);
            glDeleteVertexArrays(1, &glHandle);
            glHandle = 0;
        }
    }

    void bind()
    {
        ASSERT(created());
        GL_CHECK(glBindVertexArray(glHandle));
    }

    static void unbind()
    {
        GL_CHECK(glBindVertexArray(0));
    }

     VertexArray() = default;
    ~VertexArray() { reset(); }

    operator GLuint*() { return &glHandle; }
    operator GLuint () { return glHandle; }

    VertexArray(VertexArray&& other)            noexcept { move(other); }
    VertexArray& operator=(VertexArray&& other) noexcept { move(other); return *this; }
};
