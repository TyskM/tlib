//
// Created by Ty on 2023-01-30.
//

#pragma once

#include "IBuffer.hpp"

// Also known as an index buffer
// For storing indices
struct ElementBuffer : IBuffer
{
    template <typename T>
    void bufferData(const std::vector<T>& data, AccessType accessType)
    {
        bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(T) * data.size(), data.data(), static_cast<GLenum>(accessType));
    }

    void bind()
    {
        ASSERT(created());
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