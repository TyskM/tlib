//
// Created by Ty on 2023-01-27.
//

#pragma once

#include <TLib/Media/GL/VertexArray.hpp>
#include <TLib/Media/GL/VertexBuffer.hpp>
#include <TLib/Media/GL/Layout.hpp>
#include <TLib/Media/GL/ElementBuffer.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/NonAssignable.hpp>
#include <cstdint>

struct GPUVertexData : NonCopyable
{
protected:
    using Layout = TLib::Layout;

    VertexArray   vao; // Layout
    VertexBuffer  vbo; // Vertices
    ElementBuffer ebo; // Indices
    Layout       _layout;
    uint32_t     _vertexCount = 0;
    uint32_t     _indiceCount = 0;

    void move(GPUVertexData& src)
    {
        vao          = eastl::move(src.vao);
        vbo          = eastl::move(src.vbo);
        ebo          = eastl::move(src.ebo);
        _layout      = src._layout;
        _vertexCount = src._vertexCount;
        _indiceCount = src._indiceCount;
    }

public:
    GPUVertexData()  = default;
    ~GPUVertexData() = default;

    // Movable only
    GPUVertexData(GPUVertexData&& src)            noexcept { move(src); }
    GPUVertexData& operator=(GPUVertexData&& src) noexcept { reset(); move(src); return *this; }

    [[nodiscard]] inline const bool     valid()         const { return validLayout(); }
    [[nodiscard]] inline const bool     validLayout()   const { return vao.created() && _layout.sizeBytes() > 0; }
    [[nodiscard]] inline const bool     validVertices() const { return vbo.created(); }
    [[nodiscard]] inline const bool     validIndices()  const { return ebo.created(); }
    [[nodiscard]] inline const Layout   layout()        const { return _layout; }
    [[nodiscard]] inline const uint32_t vertexCount()   const { return _vertexCount; }
    [[nodiscard]] inline const uint32_t indiceCount()   const { return _indiceCount; }

    void reset()
    {
        ebo.reset();
        vbo.reset();
        vao.reset();
        _layout.clear();
        _vertexCount = 0;
        _indiceCount = 0;
    }

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
        if (!vao.created()) { vao.create(); }
        if (!vbo.created()) { vbo.create(); }

        rendlog->info("Setting mesh layout: Bytes={}; Size={};", layout.sizeBytes(), layout.getValues().size());

        vao.bind();
        vbo.bind();

        GLuint   index  = 0;
        uint32_t offset = 0;
        GLsizei  stride = layout.sizeBytes();

        for (const auto& l : layout.getValues())
        {
            GL_CHECK(glEnableVertexAttribArray(index));
            GL_CHECK(glVertexAttribPointer(index, l.size(), static_cast<GLenum>(l.type()),
                                           GL_FALSE, stride, (GLvoid*)(offset)));
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
        ASSERT(validLayout()); // Layout must be set before setting data

        #ifdef TLIB_DEBUG
        if (sizeof(T) != _layout.sizeBytes())
        {
            tlog::critical("The size of the value Type: ({}) Size: ({}) does not match the size of the layout ({})",
                typeid(T).name(), sizeof(T), _layout.sizeBytes());
            ASSERT(false); // Layout and data size mismatch
        }
        #endif

        if (accessType == AccessType::Static)
        { rendlog->info("Setting static mesh data: Bytes={}; Size={};", sizeof(T) * data.size(), data.size()); }

        vao.bind();
        vbo.bind();
        vbo.bufferData(data, accessType);
        _vertexCount = data.size();
    }

    template <typename ContainerOfuint32_tType>
    void setIndices(const ContainerOfuint32_tType& indices, AccessType accessType = AccessType::Static)
    {
        if (indices.size() == 0) { return; }

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