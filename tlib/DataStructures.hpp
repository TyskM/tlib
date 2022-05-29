#pragma once

#include <string>
#include <cmath>
#include "Math.hpp"

#ifdef BOOST_SERIALIZE
    #include <boost/serialization/access.hpp>
#endif

/// Vectors
template <typename T>
struct Vector2
{
    constexpr Vector2(T xv, T yv) : x{ xv }, y{ yv } { }
    constexpr Vector2() { }

    T x = 0;
    T y = 0;

    void rotate(float radians)
    { *this = rotated(radians); }

    // SIN AND COS USES RADIANS YOU DUMB IDIOT
    Vector2<T> rotated(const float& radians) const
    {
        float sinv = sin(radians);
        float cosv = cos(radians);
        return Vector2<T>(x * cosv - y * sinv, x * sinv + y * cosv);
    }

    void normalize()
    {
        *this = normalized();
    }

    Vector2<T> normalized()
    {
        Vector2<T> rv;
        float len = length();
        if (rv.x != 0) rv.x /= len;
        if (rv.y != 0) rv.y /= len;
        return rv;
    }

    float dot(const Vector2<T>& other) const
    { return x * other.x + y * other.y; }

    float cross(const Vector2<T>& other) const
    { return x * other.y - y * other.x; }

    Vector2<T> reflect(const Vector2<T>& normal) const
    { return (normal * 2) * dot(normal) - *this; }

    float length() const
    { return sqrtf(x * x + y * y); }

    float lengthSquared() const
    { return x * x + y * y; }

    Vector2<T> abs() const
    { return Vector2<T>(std::abs(x), std::abs(y)); }

    Vector2<T> distanceTo(const Vector2<T>& other) const
    { return (other - *this).abs(); }

    std::string toString() const
    { return this->operator std::string(); }

    operator std::string() const { return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ")"); }

    bool operator==(const Vector2<T> other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2<T> other) const { return !(operator==(other)); }
    Vector2<T> operator+(const Vector2<T>& other) const { return Vector2<T>(x + other.x, y + other.y); }
    Vector2<T> operator-(const Vector2<T>& other) const { return Vector2<T>(x - other.x, y - other.y); }
    Vector2<T> operator*(const Vector2<T>& other) const { return Vector2<T>(x * other.x, y * other.y); }
    Vector2<T> operator/(const Vector2<T>& other) const { return Vector2<T>(x / other.x, y / other.y); }
    Vector2<T> operator*(const int& num) const { return Vector2<T>(x * num, y * num); }
    Vector2<T> operator/(const int& num) const { return Vector2<T>(x / num, y / num); }
    Vector2<T> operator*(const float& num) const { return Vector2<T>(x * num, y * num); }
    Vector2<T> operator/(const float& num) const { return Vector2<T>(x / num, y / num); }
    Vector2<T> operator+=(const Vector2<T>& other) { return Vector2<T>(x += other.x, y += other.y); }
    Vector2<T> operator-=(const Vector2<T>& other) { return Vector2<T>(x -= other.x, y -= other.y); }
    Vector2<T> operator*=(const Vector2<T>& other) { return Vector2<T>(x *= other.x, y *= other.y); }
    Vector2<T> operator/=(const Vector2<T>& other) { return Vector2<T>(x /= other.x, y /= other.y); }
    Vector2<T> operator*=(const int& num) { return Vector2<T>(x *= num, y *= num); }
    Vector2<T> operator/=(const int& num) { return Vector2<T>(x /= num, y /= num); }
    Vector2<T> operator*=(const float& num) { return Vector2<T>(x *= num, y *= num); }
    Vector2<T> operator/=(const float& num) { return Vector2<T>(x /= num, y /= num); }

    #ifdef BOOST_SERIALIZE
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & x;
        ar & y;
    }
    #endif
};

namespace std
{
    template <typename T>
    struct hash<Vector2<T>>
    {
        size_t operator()(const Vector2<T>& k) const
        {
            const auto xhash = hash<T>()(k.x);
            const auto yhash = hash<T>()(k.y);
            return (size_t(53) + xhash) * size_t(53) + yhash;
        }
    };
}

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;

/// Colors

constexpr float uint8ToFloat(uint8_t value)
{ return value / 255.f; }

constexpr uint8_t floatToUint8(float value)
{ return value * 255.f; }

// Represents a RGBA color with values 0f-1f
struct ColorRGBAf
{
    ColorRGBAf(float rv, float gv, float bv, float av = 1.f) :
        r{ rv }, g{ gv }, b{ bv }, a{ av } { }

    float r;
    float g;
    float b;
    float a;

    static inline ColorRGBAf red() { return ColorRGBAf{ 1.f, 0.f, 0.f }; }
    static inline ColorRGBAf green() { return ColorRGBAf{ 0.f, 1.f, 0.f }; }
    static inline ColorRGBAf blue() { return ColorRGBAf{ 0.f, 0.f, 1.f }; }
    static inline ColorRGBAf white() { return ColorRGBAf{ 1.f, 1.f, 1.f }; }
    static inline ColorRGBAf black() { return ColorRGBAf{ 0.f, 0.f, 0.f }; }
    static inline ColorRGBAf purple() { return ColorRGBAf{ uint8ToFloat(127), 0.f, 1.f }; }
    static inline ColorRGBAf yellow() { return ColorRGBAf{ 1.f, 1.f, 0.f }; }
    static inline ColorRGBAf orange() { return ColorRGBAf{ 1.f, uint8ToFloat(128), 0.f }; }

#ifdef BOOST_SERIALIZE
    // Serialization
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & r;
        ar & g;
        ar & b;
        ar & a;
    }
#endif
};

// Represents a RGBA color with values 0-255
struct ColorRGBAi
{
    ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv, uint8_t av) :
        r{ rv }, g{ gv }, b{ bv }, a{ av } { }

    ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv) :
        ColorRGBAi(rv, gv, bv, 255) { }

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

struct ColorRGBi
{
    ColorRGBi(uint8_t rv, uint8_t gv, uint8_t bv) : r{ rv }, g{ gv }, b{ bv } { }
    ColorRGBi(ColorRGBAi c4) : ColorRGBi(c4.r, c4.g, c4.b) { }

    uint8_t r;
    uint8_t g;
    uint8_t b;

    operator ColorRGBAi() { return ColorRGBAi(r, g, b); }
};

/// Shapes

struct Rect
{
    Vector2f position;
    Vector2f size;

    Rect(Vector2f positionv, Vector2f sizev) : position{ positionv }, size{ sizev } { }

    Rect(float xpos, float ypos, float xwidth, float ywidth) :
        Rect(Vector2f(xpos, ypos), Vector2f(xwidth, ywidth)) { }

    Rect() { }

    bool contains(const Vector2f& pos) const
    {
        Vector2f topLeft = position;
        Vector2f bottomRight = position + size;

        return topLeft.x < pos.x && pos.x < bottomRight.x && topLeft.y < pos.y && pos.y < bottomRight.y;
    }
};
