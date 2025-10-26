//
// Created by Ty on 2023-01-27.
//

#pragma once

#include <TLib/Media/GL/GLHelpers.hpp>
#include <TLib/Types/Types.hpp>
#include <TLib/Containers/Vector.hpp>

namespace TLib
{
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
        static inline size_t _count = 0;
        
        String   _name         = "value";
        uint8_t  _size         = 0;
        GLType   _type         = GLType::Unknown;
        uint32_t _divisor      = 0;
        uint32_t _valuesPerRow = 4;
    
    public:
        constexpr Attribute(uint8_t   size,
                            GLType    type,
                            uint32_t  valuesPerRow = 4,
                            StringRef name         = "")
        {
            this->_size         = size;
            this->_type         = type;
            this->_valuesPerRow = valuesPerRow;
            if (name.empty())
            { name = fmt::format("value_{}", _count++); }
            this->_name = name;
        }
    
        inline Attribute& setDivisor(uint32_t v)
        { _divisor = v; return *this; }
    
        [[nodiscard]] inline uint32_t sizeBytes()    const { return _size * glTypeSizeMap[_type]; }
        [[nodiscard]] inline uint8_t  size()         const { return _size; }
        [[nodiscard]] inline GLType   type()         const { return _type; }
        [[nodiscard]] inline uint32_t divisor()      const { return _divisor; }
        [[nodiscard]] inline uint32_t valuesPerRow() const { return _valuesPerRow; }
    };
    
    /**
     * A collection of Attribute
     * @see: Attribute
     */
    struct Layout
    {
    protected:
        uint32_t          _sizeBytes = 0;
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
    
        [[nodiscard]] inline uint32_t                 sizeBytes() const { return _sizeBytes; }
        [[nodiscard]] inline const Vector<Attribute>& getValues() const { return _values;    }
    
        constexpr static inline Attribute Bool () { return Attribute( 1,  GLType::Bool     ); }
        constexpr static inline Attribute Int  () { return Attribute( 1,  GLType::Int      ); }
        constexpr static inline Attribute Vec2i() { return Attribute( 2,  GLType::Int      ); }
        constexpr static inline Attribute Vec3i() { return Attribute( 3,  GLType::Int      ); }
        constexpr static inline Attribute Vec4i() { return Attribute( 4,  GLType::Int      ); }
        constexpr static inline Attribute Float() { return Attribute( 1,  GLType::Float    ); }
        constexpr static inline Attribute Vec2f() { return Attribute( 2,  GLType::Float    ); }
        constexpr static inline Attribute Vec3f() { return Attribute( 3,  GLType::Float    ); }
        constexpr static inline Attribute Vec4f() { return Attribute( 4,  GLType::Float    ); }
        constexpr static inline Attribute Mat3f() { return Attribute( 9,  GLType::Float, 3 ); }
        constexpr static inline Attribute Mat4f() { return Attribute( 16, GLType::Float, 4 ); }
    };

}