//
// Created by Ty on 2023-01-27.
//

#pragma once

#include <TLib/Containers/FixedVector.hpp>
#include <TLib/Media/GL/VertexArray.hpp>
#include <TLib/Media/GL/Buffer.hpp>
#include <TLib/Media/GL/Layout.hpp>
#include <TLib/Media/Logging.hpp>
#include <TLib/NonAssignable.hpp>
#include <cstdint>

struct GPUVertexData : NonCopyable
{
protected:
    using Layout = TLib::Layout;

    void move(GPUVertexData& src)
    {
        vao     = eastl::move(src.vao);
        buffers = eastl::move(src.buffers);
        ebo     = eastl::move(src.ebo);
    }

    ArrayBuffer& getOrCreateBuffer(int32_t index)
    {
        while (buffers.size() <= index) { buffers.emplace_back(); }
        return buffers[index];
    }

public:
    VertexArray                       vao; // Layout
    ElementBuffer                     ebo; // Indices
    FixedVector<ArrayBuffer, 2, true> buffers;

    GPUVertexData()  = default;
    ~GPUVertexData() = default;

    // Movable only
    GPUVertexData(GPUVertexData&& src)            noexcept { move(src); }
    GPUVertexData& operator=(GPUVertexData&& src) noexcept { reset(); move(src); return *this; }

    void reset()
    {
        ebo.reset();
        buffers.clear();
        vao.reset();
    }

    bool bind()
    {
        vao.bind();
        return true;
    }

    static void unbind()
    { VertexArray::unbind(); }

    /**
     * @param layout The layout describing your vertices
     * @see Attribute
     */
    void setLayout(const Layout& layout, int32_t bufferIndex = 0, int32_t layoutOffset = 0)
    {
        if (!vao.created()) { vao.create(); }

        ArrayBuffer& buffer = getOrCreateBuffer(bufferIndex);
        if (!buffer.created()) { buffer.create(); }

        if constexpr(verboseRendererLogging)
            rendlog->info("Setting mesh layout: Bytes={}; Size={};", layout.sizeBytes(), layout.getValues().size());

        vao.bind();
        buffer.bind();

        GLuint  index  = layoutOffset;
        int32_t offset = 0;

        for (const auto& attribute : layout.getValues())
        {
            const int32_t attributeCount         = attribute.size();
            const int32_t attributeSizeBytes     = attribute.sizeBytes();
            const int32_t maxLayoutSlotSizeBytes = 4 * sizeof(float);

            int32_t layoutSlotsNeeded = 1;
            if (attributeCount > 4)
            {
                ASSERT(attribute.size() % attribute.valuesPerRow() == 0);
                layoutSlotsNeeded = attribute.size() / attribute.valuesPerRow();
            }
            const bool multiSlot = layoutSlotsNeeded > 1;

            for (uint32_t i = 0; i < layoutSlotsNeeded; i++)
            {
                GLint size = attributeCount;
                if (multiSlot)
                    size = layoutSlotsNeeded;

                const GLsizei stride     = layout.sizeBytes();
                const GLenum  type       = static_cast<GLenum>(attribute.type());
                const bool    normalized = false;

                GL_CHECK(glEnableVertexAttribArray(index));
                GL_CHECK(glVertexAttribPointer(index, size, type, normalized, stride, (GLvoid*)offset));
                if (attribute.divisor())
                { GL_CHECK(glVertexAttribDivisor(index, attribute.divisor())); }
                ++index;
                offset += attribute.sizeBytes() / layoutSlotsNeeded;
            }

        }

        if (layout.getValues().size() > 0)
        { ASSERT(index > 0); }

    }

    template <typename ContainerType, typename T = ContainerType::value_type>
    void setData(const ContainerType& data, AccessType accessType = AccessType::Static, int32_t bufferIndex = 0)
    {
        // Layout must be set before setting data

        // This check was nice but doesn't work with the new system. Gotta find another way to sanity check layouts
        //#ifdef TLIB_DEBUG
        //if (sizeof(T) != _layout.sizeBytes())
        //{
        //    tlog::critical("The size of the value Type: ({}) Size: ({}) does not match the size of the layout ({})",
        //        typeid(T).name(), sizeof(T), _layout.sizeBytes());
        //    ASSERT(false); // Layout and data size mismatch
        //}
        //#endif

        if constexpr(verboseRendererLogging)
            if (accessType == AccessType::Static)
                { rendlog->info("Setting static mesh data: Bytes={}; Size={};", sizeof(T) * data.size(), data.size()); }

        auto& buffer = getOrCreateBuffer(bufferIndex);

        vao.bind();
        buffer.bind();
        buffer.bufferData<ContainerType, T>(data, accessType);
    }

    template <typename ContainerType, typename T = ContainerType::value_type>
    void setIndices(const ContainerType& indices, AccessType accessType = AccessType::Static)
    {
        if (indices.size() == 0) { return; }

        if constexpr(verboseRendererLogging)
            if (accessType == AccessType::Static)
                { rendlog->info("Setting static mesh indices: Bytes={}; Size={};", indices.size() * sizeof(T), indices.size()); }
        
        if (!vao.created()) { vao.create(); }
        if (!ebo.created()) { ebo.create(); }

        vao.bind();
        ebo.bind();

        ebo.bufferData<ContainerType, T>(indices, accessType);
    }

    void removeIndices()
    { ebo.reset(); }
};