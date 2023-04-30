//
// Created by Ty on 2023-01-27.
//

#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Containers/Vector.hpp>

/**
    Used to map values to GL
    
    Example:
    struct Vector2f
    { float x, y; }
    struct Color4f
    { float r, g, b, a; }
    
    struct Vertex
    {
     Vector2f pos;
     Color4f color;
    }

    Vertex layout:
                            Position (2 floats)   Color (4 floats)
    Layout vertexLayout = { { 2, GLType::Float }, { 4, GLType::Float } }
    OR
    Layout vertexLayout = { Layout::Vec2f(), Layout::Vec4f() }

    @see Layout
 */
struct Attribute
{
protected:
    uint8_t  _size = 0;
    GLType   _type = GLType::Unknown;
    uint32_t _divisor = 0;

public:
    constexpr Attribute(uint8_t size, GLType type, uint32_t divisor = 0) : _size{ size }, _type{ type }, _divisor{ divisor }
    {
        // Attributes can't have more than 4 components
        ASSERT(size <= 4);
    }

    inline Attribute& setDivisor(uint32_t v)
    { _divisor = v; return *this; }

    [[nodiscard]] inline uint32_t sizeBytes() const { return _size * glTypeSizeMap[_type]; }
    [[nodiscard]] inline uint8_t  size()      const { return _size; }
    [[nodiscard]] inline GLType   type()      const { return _type; }
    [[nodiscard]] inline uint32_t divisor()   const { return _divisor; }
};

/**
 * A collection of Attribute
 * @see: Attribute
 */
struct Layout
{
protected:
    uint32_t _sizeBytes = 0;
    Vector<Attribute> _values;

public:
    Layout() = default;

    Layout(const std::initializer_list<Attribute>& values) { set(values); }

    Layout(const Attribute& attr, uint32_t count) { set(attr, count); }

    void set(const Attribute& attr, const uint32_t count)
    {
        clear();
        _values.reserve(count);
        for (int i = 0; i < count; ++i)
        { append(attr); }
    }

    void set(const std::initializer_list<Attribute>& values)
    {
        clear();
        _values.reserve(values.size());
        for (auto& l : values)
        { append(l); }
    }

    Layout& append(const Attribute& attr, const uint32_t count)
    {
        for (int i = 0; i < count; ++i)
        {
            _sizeBytes += attr.sizeBytes();
            _values.push_back(attr);
        }
        return *this;
    }

    Layout& append(const Attribute& attr)
    {
        _sizeBytes += attr.sizeBytes();
        _values.push_back(attr);
        return *this;
    }

    Layout& append(const Layout& layout)
    {
        for (auto& attr : layout.getValues())
        { append(attr); }
        return *this;
    }

    void clear()
    {
        _sizeBytes = 0;
        _values.clear();
    }

    Layout& setDivisor(uint32_t v)
    {
        for (auto& attr : _values)
        { attr.setDivisor(v); }
        return *this;
    }

    [[nodiscard]] inline uint32_t sizeBytes() const { return _sizeBytes; }
    [[nodiscard]] inline const Vector<Attribute>& getValues() const { return _values; }

    // TODO: low prio, add more presets. Most of the presets that matter are already here
    [[maybe_unused]] constexpr static inline Attribute Bool () { return { 1, GLType::Bool  }; }
    [[maybe_unused]] constexpr static inline Attribute Float() { return { 1, GLType::Float }; }
    [[maybe_unused]] constexpr static inline Attribute Int  () { return { 1, GLType::Int   }; }
    [[maybe_unused]] constexpr static inline Attribute Vec2f() { return { 2, GLType::Float }; }
    [[maybe_unused]] constexpr static inline Attribute Vec3f() { return { 3, GLType::Float }; }
    [[maybe_unused]] constexpr static inline Attribute Vec4f() { return { 4, GLType::Float }; }

    [[maybe_unused]] static inline Layout Mat3f() { return { Vec3f(), 3 }; }
    [[maybe_unused]] static inline Layout Mat4f() { return { Vec4f(), 4 }; }
};
