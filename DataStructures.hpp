#pragma once

#include <string>
#include <cmath>
#include "Math.hpp"

// There's always a min/max macro somewhere. Make it stop!!!
#undef min
#undef max

#pragma region Vector2
/// Container with 2 of a single type
template <typename T>
struct Vector2
{
    constexpr Vector2(T xv, T yv) : x{ xv }, y{ yv } { }
    constexpr Vector2() { }

    template <typename CT>
    constexpr explicit Vector2(const CT& other)
    {
        x = static_cast<T>(other.x);
        y = static_cast<T>(other.y);
    }

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

    Vector2<T> rotated(const float& radians, Vector2<T> origin) const
    {
        float sinv = sin(radians);
        float cosv = cos(radians);
        const auto nvec = *this;
        nvec -= origin;
        return nvec.rotated(radians) + origin;
    }

    void normalize()
    {
        *this = normalized();
    }

    Vector2<T> normalized()
    {
        Vector2<T> rv = *this;
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

    T distanceTo(const Vector2<T>& other) const
    { return std::max( std::abs( x - other.x), std::abs( y - other.y) ); }

    std::string toString() const
    { return this->operator std::string(); }

    Vector2<T> relMultiply(const Vector2<T> other, T amount) { return (*this - other) * amount + other; }

    Vector2<T> floored()    const { return { floor(x), floor(y) }; }
    Vector2<T> ceiled()     const { return { ceil(x),  ceil(y)  }; }
    Vector2<T> rounded()    const { return { round(x), round(y) }; }
    Vector2<T> abs()        const { return { std::abs(x), std::abs(y)   }; }
    Vector2<T> sqrt()       const { return { std::sqrt(x), std::sqrt(y) }; }
    Vector2<T> pow(T value) const { return { std::pow(x, value), std::pow(y, value) }; }

    operator std::string() const { return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ")"); }

    bool operator==(const Vector2<T> other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2<T> other) const { return !(operator==(other)); }
    Vector2<T> operator-() const { return Vector2<T>(-x, -y); }
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
};

///\cond INTERNAL
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
///\endcond

/// Container with 2 floats
/// \relates Vector2
using Vector2f = Vector2<float>;

/// Container with 2 ints
/// \relates Vector2
using Vector2i = Vector2<int>;

using Vector2i8 = Vector2<int8_t>;
#pragma endregion

#pragma region Vector3

template <typename T>
struct Vector3
{
    constexpr Vector3(T xv, T yv, T zv) : x{ xv }, y{ yv }, z{ zv } { }
    constexpr Vector3() { }

    template <typename CT>
    constexpr explicit Vector3(const CT& other)
    {
        x = static_cast<T>(other.x);
        y = static_cast<T>(other.y);
        z = static_cast<T>(other.z);
    }

    T x = 0;
    T y = 0;
    T z = 0;

    std::string toString() const
    { return this->operator std::string(); }

    Vector2<T> floored()    const { return { floor(x), floor(y), floor(z) }; }
    Vector2<T> ceiled()     const { return { ceil(x),  ceil(y),  ceil(z)  }; }
    Vector2<T> rounded()    const { return { round(x), round(y), round(z) }; }
    Vector2<T> abs()        const { return { std::abs(x),  std::abs(y),  std::abs(z)  }; }
    Vector2<T> sqrt()       const { return { std::sqrt(x), std::sqrt(y), std::sqrt(z) }; }
    Vector2<T> pow(T value) const { return { std::pow(x, value), std::pow(y, value), std::pow(z, value) }; }

    operator std::string() const
    { return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")"); }

    bool operator==(const Vector3<T> other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Vector3<T> other) const { return !(operator==(other)); }
    Vector3<T> operator-() const { return Vector3<T>(-x, -y, -z); }
    Vector3<T> operator+(const Vector3<T>& other) const { return Vector3<T>(x + other.x, y + other.y, z + other.z); }
    Vector3<T> operator-(const Vector3<T>& other) const { return Vector3<T>(x - other.x, y - other.y, z - other.z); }
    Vector3<T> operator*(const Vector3<T>& other) const { return Vector3<T>(x * other.x, y * other.y, z * other.z); }
    Vector3<T> operator/(const Vector3<T>& other) const { return Vector3<T>(x / other.x, y / other.y, z / other.z); }
    Vector3<T> operator*(const int& num) const { return Vector3<T>(x * num, y * num, z * num); }
    Vector3<T> operator/(const int& num) const { return Vector3<T>(x / num, y / num, z / num); }
    Vector3<T> operator*(const float& num) const { return Vector3<T>(x * num, y * num, z * num); }
    Vector3<T> operator/(const float& num) const { return Vector3<T>(x / num, y / num, z / num); }
    Vector3<T> operator+=(const Vector3<T>& other) { return Vector3<T>(x += other.x, y += other.y, z += other.z); }
    Vector3<T> operator-=(const Vector3<T>& other) { return Vector3<T>(x -= other.x, y -= other.y, z -= other.z); }
    Vector3<T> operator*=(const Vector3<T>& other) { return Vector3<T>(x *= other.x, y *= other.y, z *= other.z); }
    Vector3<T> operator/=(const Vector3<T>& other) { return Vector3<T>(x /= other.x, y /= other.y, z /= other.z); }
    Vector3<T> operator*=(const int& num) { return Vector3<T>(x *= num, y *= num, z *= num); }
    Vector3<T> operator/=(const int& num) { return Vector3<T>(x /= num, y /= num, z /= num); }
    Vector3<T> operator*=(const float& num) { return Vector3<T>(x *= num, y *= num, z *= num); }
    Vector3<T> operator/=(const float& num) { return Vector3<T>(x /= num, y /= num, z /= num); }
};

using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;

#pragma endregion

constexpr float uint8ToFloat(uint8_t value)
{ return value / 255.f; }

constexpr uint8_t floatToUint8(float value)
{ return value * 255.f; }

// Represents a RGBA color with values 0f-1f
struct ColorRGBAf
{
    constexpr ColorRGBAf() = default;
    constexpr ColorRGBAf(float rv, float gv, float bv, float av = 1.f) :
        r{ rv }, g{ gv }, b{ bv }, a{ av } { }

    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;

    bool operator==(const ColorRGBAf& other) { return r == other.r && g == other.g && b == other.b && a == other.a; }
    bool operator!=(const ColorRGBAf& other) { return !(operator==(other)); }

    static constexpr inline ColorRGBAf red()    { return ColorRGBAf{ 1.f, 0.f, 0.f }; }
    static constexpr inline ColorRGBAf green()  { return ColorRGBAf{ 0.f, 1.f, 0.f }; }
    static constexpr inline ColorRGBAf blue()   { return ColorRGBAf{ 0.f, 0.f, 1.f }; }
    static constexpr inline ColorRGBAf white()  { return ColorRGBAf{ 1.f, 1.f, 1.f }; }
    static constexpr inline ColorRGBAf black()  { return ColorRGBAf{ 0.f, 0.f, 0.f }; }
    static constexpr inline ColorRGBAf purple() { return ColorRGBAf{ uint8ToFloat(127), 0.f, 1.f }; }
    static constexpr inline ColorRGBAf yellow() { return ColorRGBAf{ 1.f, 1.f, 0.f }; }
    static constexpr inline ColorRGBAf orange() { return ColorRGBAf{ 1.f, uint8ToFloat(128), 0.f }; }
};

// Represents a RGBA color with values 0-255
struct ColorRGBAi
{
    ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv, uint8_t av) :
        r{ rv }, g{ gv }, b{ bv }, a{ av } { }

    ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv) :
        ColorRGBAi(rv, gv, bv, 255) { }

    ColorRGBAi(const ColorRGBAf& cf) : r{floatToUint8(cf.r)}, g{floatToUint8(cf.g)}, b{floatToUint8(cf.b)}, a{floatToUint8(cf.a)} {  }

    ColorRGBAi() { }

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;
};

struct ColorRGBi
{
    ColorRGBi(uint8_t rv, uint8_t gv, uint8_t bv) : r{ rv }, g{ gv }, b{ bv } { }
    ColorRGBi(ColorRGBAi c4) : ColorRGBi(c4.r, c4.g, c4.b) { }
    ColorRGBi() : r{ 0 }, g{ 0 }, b{ 0 } { }

    uint8_t r;
    uint8_t g;
    uint8_t b;

    operator ColorRGBAi() { return ColorRGBAi(r, g, b); }
};

template<typename T = float>
struct Circle
{
    T x, y, radius;

    constexpr Circle(T x, T y, T radius) : x{ x }, y{ y }, radius{ radius } { }

    bool intersects(const Circle<T>& other)
    {
        return Vector2<T>(x, y).distanceTo(Vector2<T>(other.x, other.y)) < other.radius + radius;
    }
};

using Circlef = Circle<float>;

template<typename T = float>
struct Rect
{
    T x, y, width, height;

    Rect(Vector2<T> pos, Vector2<T> size) : x{ pos.x }, y{ pos.y }, width{ size.x }, height{ size.y } { }
    Rect(T x, T y, T width, T height) : x{ x }, y{ y }, width{ width }, height{ height } { }
    Rect() { }

    bool contains(const T& x, const T& y) const
    {
        Vector2<T> topLeft = { this->x, this->y };
        Vector2<T> bottomRight = topLeft + Vector2<T>{ width, height };

        return topLeft.x < x && x < bottomRight.x && topLeft.y < y && y < bottomRight.y;
    }

    bool intersects(const Rect<T>& otherRect)
    {
        Vector2<T> bottomRight = Vector2<T>{ x, y } + Vector2<T>{ width, height };
        Vector2<T> otherBottomRight = Vector2<T>{ otherRect.x, otherRect.y } +
                                      Vector2<T>{ otherRect.width, otherRect.height };

        T x1, x2, y1, y2;
        T ox1, ox2, oy1, oy2;

        x1 = x; y1 = y;
        x2 = x + width;
        y2 = y + height;

        ox1 = otherRect.x;
        oy1 = otherRect.y;
        ox2 = otherRect.x + otherRect.width;
        oy2 = otherRect.y + otherRect.height;

        return !(x1  >= ox2 ||
                 ox1 >= x2  ||
                 y1  >= oy2 ||
                 oy1 >= y2);
    }

    bool intersects(const Circle<T>& circle)
    {
        // https://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection
        // Find the closest point to the circle within the rectangle
        float closestX = math::clamp(circle.x, x, x + width);
        float closestY = math::clamp(circle.y, y, y + height);

        // Calculate the distance between the circle's center and this closest point
        float distanceX = circle.x - closestX;
        float distanceY = circle.y - closestY;

        // If the distance is less than the circle's radius, an intersection occurs
        float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
        return distanceSquared <= (circle.radius * circle.radius);
    }

    std::string toString()
    {
        return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(width) + ", " + std::to_string(height) + ")");
    }
};

using Rectf = Rect<float>;
using Recti = Rect<int>;