#pragma once

#include "IBuffer.hpp"

// Used for storing vertices
// You probably also want a VertexArray
struct VertexBuffer : IBuffer
{
    template <typename ContainerType, typename T = ContainerType::value_type>
    void bufferData(const ContainerType& data, AccessType accessType)
    {
        bind();
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), static_cast<GLenum>(accessType)));
    }

    void bind()
    {
        ASSERT(created());
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, glHandle));
        glState.boundVertexBuffer = this;
    }

    static void unbind()
    {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        glState.boundVertexBuffer = nullptr;
    }
};
