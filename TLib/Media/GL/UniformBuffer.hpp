//
// Created by Ty on 2023-01-30.
//

#pragma once

#include "IBuffer.hpp"

struct UniformBuffer : IBuffer
{
    template <typename T>
    void bufferData(const std::vector<T>& data, AccessType accessType)
    {
        bind();
        glBufferData(GL_UNIFORM_BUFFER, sizeof(T) * data.size(), data.data(), static_cast<GLenum>(accessType));
    }

    void bind()
    {
        ASSERT(created());
        if (glState.boundUniformBuffer == this) { return; }
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, glHandle));
        glState.boundUniformBuffer = this;
    }

    static void unbind()
    {
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
        glState.boundUniformBuffer = nullptr;
    }

    void setBufferBase(int index = 0)
    {
        bind();
        glBindBufferBase(GL_UNIFORM_BUFFER, index, glHandle);
    }
};