//
// Created by Ty on 2023-01-27.
//

#pragma once

#include <TLib/Media/GL/VertexArray.hpp>
#include <TLib/Media/GL/VertexBuffer.hpp>
#include <TLib/Media/GL/Layout.hpp>
#include <TLib/Media/GL/ElementBuffer.hpp>
#include <TLib/Media/Logging.hpp>
#include <cstdint>

struct Mesh
{
protected:
    VertexArray   vao; // Layout
    VertexBuffer  vbo; // Vertices
    ElementBuffer ebo; // Indices (optional)
    Layout        _layout;
    uint32_t      _vertexCount = 0;
    uint32_t      _indiceCount = 0;

public:
    Mesh() = default;

    [[nodiscard]] inline bool valid()                 const { return validLayout(); }
    [[nodiscard]] inline bool validLayout()           const { return vao.created() && _layout.sizeBytes() > 0; }
    [[nodiscard]] inline bool validVertices()         const { return vbo.created(); }
    [[nodiscard]] inline bool validIndices()          const { return ebo.created(); }
    [[nodiscard]] inline const Layout   layout()      const { return _layout; }
    [[nodiscard]] inline const uint32_t vertexCount() const { return _vertexCount; }
    [[nodiscard]] inline const uint32_t indiceCount() const { return _indiceCount; }

    bool bind()
    {
        if (!valid()) { return false; }
        vao.bind();
        return true;
    }

    static void unbind()
    { VertexArray::unbind(); }

    /**
     * @param layout The layout describing your vertices
     * @see Attribute
     */
    void setLayout(const Layout& layout)
    {
        rendlog->info("Setting mesh layout: Bytes={}; Size={};", layout.sizeBytes(), layout.getValues().size());

        if (!vao.created()) { vao.create(); }
        if (!vbo.created()) { vbo.create(); }

        vao.bind();
        vbo.bind();

        int index = 0;
        int offset = 0;
        uint32_t stride = layout.sizeBytes();

        for (auto& l : layout.getValues())
        {
            GL_CHECK(glVertexAttribPointer(index, l.size(), static_cast<GLenum>(l.type()), GL_FALSE, stride, (GLvoid*)offset));
            GL_CHECK(glEnableVertexAttribArray(index));
            if (l.divisor())
            { GL_CHECK(glVertexAttribDivisor(index, l.divisor())); }
            ++index;
            offset += l.sizeBytes();
        }

        _layout = layout;

        if (layout.getValues().size() > 0)
        { ASSERT(index > 0); }

    }

    template <typename ContainerType>
    void setData(const ContainerType& data, AccessType accessType = AccessType::Static)
    {
        using T = ContainerType::value_type;
        // Layout must be set before setting data
        ASSERT(validLayout());

        // Layout and data size mismatch
        ASSERT(sizeof(T) == _layout.sizeBytes()); 

        if (accessType == AccessType::Static)
        { rendlog->info("Setting static mesh data: Bytes={}; Size={};", sizeof(T) * data.size(), data.size()); }

        vao.bind();
        vbo.bind();
        vbo.bufferData(data, accessType);
        _vertexCount = data.size();
    }

    // Indices are not required, but they are nice :)
    template <typename ContainerOfuint32_tType>
    void setIndices(const ContainerOfuint32_tType& indices, AccessType accessType = AccessType::Static)
    {
        if (accessType == AccessType::Static)
        { rendlog->info("Setting static mesh indices: Bytes={}; Size={};", indices.size() * sizeof(uint32_t), indices.size()); }
        if (!vao.created()) { vao.create(); }
        if (!ebo.created()) { ebo.create(); }

        vao.bind();
        ebo.bind();

        ebo.bufferData(indices, accessType);
        _indiceCount = indices.size();

        ASSERT(_indiceCount > 0);
    }

    void removeIndices()
    {
        _indiceCount = 0;
        ebo.reset();
    }
};