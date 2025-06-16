
#pragma once

#include <TLib/Macros.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/Media/GL/GLHelpers.hpp>

enum class AccessType : GLenum
{
    Static  [[maybe_unused]] = GL_STATIC_DRAW,
    Dynamic [[maybe_unused]] = GL_DYNAMIC_DRAW,
    Stream  [[maybe_unused]] = GL_STREAM_DRAW
};

template <GLenum Type>
struct Buffer
{
private:
    using This = Buffer<Type>;

    GLuint glHandle = 0;
    size_t _size    = 0;

    void move(This& other)
    {
        _size          = other._size;
        glHandle       = other.glHandle;
        other.glHandle = 0;
    }

public:

    DISABLE_COPY(Buffer);

     Buffer() = default;
    ~Buffer() { reset(); }

    template <typename ContainerType, typename T = ContainerType::value_type>
    void bufferData(const ContainerType& data, AccessType accessType)
    {
        bind();
        GL_CHECK(glBufferData(Type, sizeof(T) * data.size(), data.data(), static_cast<GLenum>(accessType)));
        _size = data.size();
    }

    void bind()
    {
        ASSERT(created());
        GL_CHECK(glBindBuffer(Type, glHandle));
    }

    static void unbind()
    {
        GL_CHECK(glBindBuffer(Type, 0));
    }

    size_t size() const { return _size; }

    bool created() const { return glHandle != 0; }

    void create()
    {
        reset();
        GL_CHECK(glGenBuffers(1, &glHandle));

        if constexpr(verboseRendererLogging)
            rendlog->info("Created buffer at location {}", glHandle);
    }

    void reset()
    {
        if (created())
        {
            if constexpr(verboseRendererLogging)
                rendlog->info("Destroyed buffer at location {}", glHandle);
            glDeleteBuffers(1, &glHandle);
            glHandle = 0;
        }
        _size = 0;
    }

    void setBufferBase(int32_t index)
    {
        ASSERT(Type == GL_UNIFORM_BUFFER);
        glBindBufferBase(GL_UNIFORM_BUFFER, index, glHandle);
    }

    operator GLuint*() { return &glHandle; }
    operator GLuint () { return  glHandle; }

    Buffer(This&& other)            noexcept { move(other); }
    Buffer& operator=(This&& other) noexcept { move(other); return *this; }
};

using ArrayBuffer   = Buffer<GL_ARRAY_BUFFER>;
using ElementBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER>;
using UniformBuffer = Buffer<GL_UNIFORM_BUFFER>;
