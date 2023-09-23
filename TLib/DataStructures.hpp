#pragma once

#include <string>
#include <cmath>
#include <limits>
#include <TLib/String.hpp>
#include <TLib/Math.hpp>

// There's always a min/max macro somewhere. Make it stop!!!
#undef min
#undef max

constexpr float uint8ToFloat(uint8_t value)
{ return value / 255.f; }

constexpr uint8_t floatToUint8(float value)
{ return static_cast<uint8_t>(value * 255.f); }

template <typename T>
struct Vector2
{
    using value_type = T;

    constexpr Vector2(T xv, T yv) : x{ xv }, y{ yv } { }
    constexpr Vector2(T v) : x{ v }, y{ v } {}
    constexpr Vector2() = default;

    template <typename CT>
    constexpr explicit Vector2(const CT& other)
    {
        x = static_cast<T>(other.x);
        y = static_cast<T>(other.y);
    }

    T x = 0;
    T y = 0;

    // In radians
    inline T angle() const
    { return atan2(y, x); }

    // In radians
    T angleTo(const Vector2<T>& other) const
    { return atan2(cross(other), dot(other)); }

    // In radians
    T angleToRel(const Vector2<T>& other) const
    { return (other - *this).angle(); }

    void rotate(T radians)
    { *this = rotated(radians); }

    // SIN AND COS USES RADIANS YOU DUMB IDIOT
    Vector2<T> rotated(const T radians) const
    {
        T sinv = static_cast<T>(sin(radians));
        T cosv = static_cast<T>(cos(radians));
        return Vector2<T>(x * cosv - y * sinv, x * sinv + y * cosv);
    }

    Vector2<T> rotated(const T radians, const Vector2<T>& origin) const
    {
        T sinv = static_cast<T>(sin(radians));
        T cosv = static_cast<T>(cos(radians));
        auto nvec = *this;
        nvec -= origin;
        return nvec.rotated(radians) + origin;
    }

    void normalize()
    {
        *this = normalized();
    }

    Vector2<T> normalized() const
    {
        Vector2<T> rv = *this;
        T len = length();
        if (rv.x != 0) rv.x /= len;
        if (rv.y != 0) rv.y /= len;
        return rv;
    }

    T dot(const Vector2<T>& other) const
    { return x * other.x + y * other.y; }

    T cross(const Vector2<T>& other) const
    { return x * other.y - y * other.x; }

    Vector2<T> reflect(const Vector2<T>& normal) const
    { return (normal * 2) * dot(normal) - *this; }

    T length() const
    { return static_cast<T>(std::sqrt(x * x + y * y)); }

    T lengthSquared() const
    { return x * x + y * y; }

    T distanceTo(const Vector2<T>& other) const
    { return static_cast<T>(std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y))); }

    T distanceToSquared(const Vector2<T>& other) const
    { return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y); }

    Vector2<T> relMultiply(const Vector2<T> other, T amount)
    { return (*this - other) * amount + other; }

    [[nodiscard]]
    bool isNan() const
    { return isnan(x) || isnan(y); }

    String toString() const
    { return this->operator String(); }

    Vector2<T> floored()    const { return Vector2<T>( static_cast<T>(floor(x)),            static_cast<T>(floor(y))           ); }
    Vector2<T> ceiled()     const { return Vector2<T>( static_cast<T>(ceil(x)),             static_cast<T>(ceil(y))            ); }
    Vector2<T> rounded()    const { return Vector2<T>( static_cast<T>(round(x)),            static_cast<T>(round(y))           ); }
    Vector2<T> abs()        const { return Vector2<T>( static_cast<T>(std::abs(x)),         static_cast<T>(std::abs(y))        ); }
    Vector2<T> sqrt()       const { return Vector2<T>( static_cast<T>(std::sqrt(x)),        static_cast<T>(std::sqrt(y))       ); }
    Vector2<T> pow(T value) const { return Vector2<T>( static_cast<T>(std::pow(x, value)),  static_cast<T>(std::pow(y, value)) ); }

    operator   String()                             const { return String("(" + std::to_string(x) + ", " + std::to_string(y) + ")"); }
    bool       operator==(const Vector2<T>& other)  const { return x == other.x && y == other.y; }
    bool       operator!=(const Vector2<T>& other)  const { return !(operator==(other)); }
    Vector2<T> operator-()                          const { return Vector2<T>(-x, -y); }
    Vector2<T> operator+(const Vector2<T>& other)   const { return Vector2<T>(x + other.x, y + other.y); }
    Vector2<T> operator-(const Vector2<T>& other)   const { return Vector2<T>(x - other.x, y - other.y); }
    Vector2<T> operator*(const Vector2<T>& other)   const { return Vector2<T>(x * other.x, y * other.y); }
    Vector2<T> operator/(const Vector2<T>& other)   const { return Vector2<T>(x / other.x, y / other.y); }
    Vector2<T> operator*(const int& num)            const { return Vector2<T>(x * num, y * num); }
    Vector2<T> operator/(const int& num)            const { return Vector2<T>(x / num, y / num); }
    Vector2<T> operator+(const float& num)          const { return Vector2<T>(x + num, y + num); }
    Vector2<T> operator-(const float& num)          const { return Vector2<T>(x - num, y - num); }
    Vector2<T> operator*(const float& num)          const { return Vector2<T>(x * num, y * num); }
    Vector2<T> operator/(const float& num)          const { return Vector2<T>(x / num, y / num); }
    Vector2<T> operator+=(const Vector2<T>& other)        { return Vector2<T>(x += other.x, y += other.y); }
    Vector2<T> operator-=(const Vector2<T>& other)        { return Vector2<T>(x -= other.x, y -= other.y); }
    Vector2<T> operator*=(const Vector2<T>& other)        { return Vector2<T>(x *= other.x, y *= other.y); }
    Vector2<T> operator/=(const Vector2<T>& other)        { return Vector2<T>(x /= other.x, y /= other.y); }
    Vector2<T> operator*=(const int& num)                 { return Vector2<T>(x *= num, y *= num); }
    Vector2<T> operator/=(const int& num)                 { return Vector2<T>(x /= num, y /= num); }
    Vector2<T> operator*=(const float& num)               { return Vector2<T>(x *= num, y *= num); }
    Vector2<T> operator/=(const float& num)               { return Vector2<T>(x /= num, y /= num); }

};
using Vector2f  = Vector2<float>;
using Vector2i  = Vector2<int>;
using Vector2i8 = Vector2<int8_t>;

// Support hashing
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

    bool operator==(const Vector3<T> other)       const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Vector3<T> other)       const { return !(operator==(other)); }
    Vector3<T> operator-()                        const { return Vector3<T>(-x, -y, -z); }
    Vector3<T> operator+(const Vector3<T>& other) const { return Vector3<T>(x + other.x, y + other.y, z + other.z); }
    Vector3<T> operator-(const Vector3<T>& other) const { return Vector3<T>(x - other.x, y - other.y, z - other.z); }
    Vector3<T> operator*(const Vector3<T>& other) const { return Vector3<T>(x * other.x, y * other.y, z * other.z); }
    Vector3<T> operator/(const Vector3<T>& other) const { return Vector3<T>(x / other.x, y / other.y, z / other.z); }
    Vector3<T> operator*(const int& num)          const { return Vector3<T>(x * num, y * num, z * num); }
    Vector3<T> operator/(const int& num)          const { return Vector3<T>(x / num, y / num, z / num); }
    Vector3<T> operator*(const float& num)        const { return Vector3<T>(x * num, y * num, z * num); }
    Vector3<T> operator/(const float& num)        const { return Vector3<T>(x / num, y / num, z / num); }
    Vector3<T> operator+=(const Vector3<T>& other)      { return Vector3<T>(x += other.x, y += other.y, z += other.z); }
    Vector3<T> operator-=(const Vector3<T>& other)      { return Vector3<T>(x -= other.x, y -= other.y, z -= other.z); }
    Vector3<T> operator*=(const Vector3<T>& other)      { return Vector3<T>(x *= other.x, y *= other.y, z *= other.z); }
    Vector3<T> operator/=(const Vector3<T>& other)      { return Vector3<T>(x /= other.x, y /= other.y, z /= other.z); }
    Vector3<T> operator*=(const int& num)               { return Vector3<T>(x *= num, y *= num, z *= num); }
    Vector3<T> operator/=(const int& num)               { return Vector3<T>(x /= num, y /= num, z /= num); }
    Vector3<T> operator*=(const float& num)             { return Vector3<T>(x *= num, y *= num, z *= num); }
    Vector3<T> operator/=(const float& num)             { return Vector3<T>(x /= num, y /= num, z /= num); }

};
using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;

template <typename T>
struct Vector4
{
    using value_type = T;

    constexpr Vector4(T xv, T yv, T zv, T wv) : x{ xv }, y{ yv }, z{ zv }, w{ wv } { }
    constexpr Vector4() { }

    T x = 0;
    T y = 0;
    T z = 0;
    T w = 0;
};

// Represents a RGBA color with values 0f-1f
struct ColorRGBAf
{
    constexpr ColorRGBAf() = default;

    constexpr ColorRGBAf(float rv, float gv, float bv, float av = 1.f) :
        r{ rv }, g{ gv }, b{ bv }, a{ av } { }

    explicit constexpr ColorRGBAf(int rv, int gv, int bv, int av = 255) :
        r{ uint8ToFloat(rv) }, g{ uint8ToFloat(gv) }, b{ uint8ToFloat(bv) }, a{ uint8ToFloat(av) } { }

    float r = 0;
    float g = 0;
    float b = 0;
    float a = 0;

    bool operator==(const ColorRGBAf& other) { return r == other.r && g == other.g && b == other.b && a == other.a; }
    bool operator!=(const ColorRGBAf& other) { return !(operator==(other)); }

    static constexpr inline ColorRGBAf red()         { return ColorRGBAf{ 255, 0,   0    }; }
    static constexpr inline ColorRGBAf green()       { return ColorRGBAf{ 0,   255, 0    }; }
    static constexpr inline ColorRGBAf blue()        { return ColorRGBAf{ 0,   0,   255  }; }
    static constexpr inline ColorRGBAf white()       { return ColorRGBAf{ 255, 255, 255  }; }
    static constexpr inline ColorRGBAf black()       { return ColorRGBAf{ 0,   0,   0    }; }
    static constexpr inline ColorRGBAf purple()      { return ColorRGBAf{ 127, 0,   255  }; }
    static constexpr inline ColorRGBAf yellow()      { return ColorRGBAf{ 255, 255, 0    }; }
    static constexpr inline ColorRGBAf orange()      { return ColorRGBAf{ 255, 128, 0    }; }
    static constexpr inline ColorRGBAf magenta()     { return ColorRGBAf{ 255, 0,   255  }; }
    static constexpr inline ColorRGBAf darkMagenta() { return ColorRGBAf{ 139, 0,   139  }; }
    static constexpr inline ColorRGBAf lime()        { return ColorRGBAf{ 0,   255, 0    }; }
    static constexpr inline ColorRGBAf cyan()        { return ColorRGBAf{ 0,   255, 255  }; }
    static constexpr inline ColorRGBAf steelBlue()   { return ColorRGBAf{ 70,  130, 180  }; }
    static constexpr inline ColorRGBAf royalBlue()   { return ColorRGBAf{ 65,  105, 225  }; }
    static constexpr inline ColorRGBAf gold()        { return ColorRGBAf{ 255, 215, 0    }; }
};

// Represents a RGB color with values 0f-1f
struct ColorRGBf
{
    constexpr ColorRGBf() = default;

    constexpr ColorRGBf(float rv, float gv, float bv) :
        r{rv}, g{gv}, b{bv} { }

    explicit constexpr ColorRGBf(int rv, int gv, int bv) :
        r{uint8ToFloat(rv)}, g{uint8ToFloat(gv)}, b{uint8ToFloat(bv)} { }

    explicit constexpr ColorRGBf(const ColorRGBAf& rgba) :
        r{rgba.r}, g{rgba.g}, b{rgba.b} { }

    float r = 0;
    float g = 0;
    float b = 0;

    bool operator==(const ColorRGBf& other) { return r == other.r && g == other.g && b == other.b; }
    bool operator!=(const ColorRGBf& other) { return !(operator==(other)); }
};

// Represents a RGBA color with values 0-255
struct ColorRGBAi
{
    constexpr ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv, uint8_t av) :
                              r{ rv },    g{ gv },    b{ bv },    a{ av } { }

    constexpr ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv) : ColorRGBAi(rv, gv, bv, 255) { }

    constexpr ColorRGBAi(const ColorRGBAf& cf) : r{floatToUint8(cf.r)}, g{floatToUint8(cf.g)}, b{floatToUint8(cf.b)}, a{floatToUint8(cf.a)} {  }

    constexpr ColorRGBAi() { }

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    static constexpr inline ColorRGBAi red()    { return ColorRGBAf::red();    }
    static constexpr inline ColorRGBAi green()  { return ColorRGBAf::green();  }
    static constexpr inline ColorRGBAi blue()   { return ColorRGBAf::blue();   }
    static constexpr inline ColorRGBAi white()  { return ColorRGBAf::white();  }
    static constexpr inline ColorRGBAi black()  { return ColorRGBAf::black();  }
    static constexpr inline ColorRGBAi purple() { return ColorRGBAf::purple(); }
    static constexpr inline ColorRGBAi yellow() { return ColorRGBAf::yellow(); }
    static constexpr inline ColorRGBAi orange() { return ColorRGBAf::orange(); }
};

struct ColorRGBi
{
    constexpr ColorRGBi(uint8_t rv, uint8_t gv, uint8_t bv) : r{ rv }, g{ gv }, b{ bv } { }
    constexpr ColorRGBi(ColorRGBAi c4) : ColorRGBi(c4.r, c4.g, c4.b) { }
    constexpr ColorRGBi() : r{ 0 }, g{ 0 }, b{ 0 } { }

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
    { return Vector2<T>(x, y).distanceTo(Vector2<T>(other.x, other.y)) < other.radius + radius; }

    bool contains(T px, T py)
    { return std::pow(px-x, 2) + std::pow(py-y, 2) < std::pow(radius, 2); }

    bool contains(const Vector2<T>& v)
    { return contains(v.x, v.y); }
};

using Circlef = Circle<float>;
using Circlei = Circle<int>;

template<typename T = float>
struct Rect
{
    T x      = 0;
    T y      = 0;
    T width  = 0;
    T height = 0;

    Rect(Vector2<T> pos, Vector2<T> size) : x{ pos.x }, y{ pos.y }, width{ size.x }, height{ size.y } { }
    Rect(T x, T y, T width, T height) : x{ x }, y{ y }, width{ width }, height{ height } { }
    Rect() = default;

    template <typename CT>
    constexpr explicit Rect(const CT& other)
    {
        x      = static_cast<T>(other.x);
        y      = static_cast<T>(other.y);
        width  = static_cast<T>(other.width);
        height = static_cast<T>(other.height);
    }

    Vector2<T> getPos() const
    { return Vector2<T>(x, y); }

    Vector2<T> getSize() const
    { return Vector2<T>(width, height); }

    T getRight() const
    { return x + width; }

    T getBottom() const
    { return y + height; }

    Vector2<T> getRightBottom() const
    { return { getRight(), getBottom() }; }

    Vector2<T> getTopRight() const
    { return { getRight(), y }; }

    Vector2<T> getBottomLeft() const
    { return { x, getBottom() }; }

    bool contains(Vector2<T> value) const
    { return contains(value.x, value.y); }

    bool contains(const T& x, const T& y) const
    {
        Vector2<T> topLeft = { this->x, this->y };
        Vector2<T> bottomRight = topLeft + Vector2<T>{ width, height };

        return topLeft.x < x && x < bottomRight.x && topLeft.y < y && y < bottomRight.y;
    }

    bool intersects(const Rect<T>& otherRect) const
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
        float closestX = math::clamp<T>(circle.x, x, x + width);
        float closestY = math::clamp<T>(circle.y, y, y + height);

        // Calculate the distance between the circle's center and this closest point
        float distanceX = circle.x - closestX;
        float distanceY = circle.y - closestY;

        // If the distance is less than the circle's radius, an intersection occurs
        float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
        return distanceSquared <= (circle.radius * circle.radius);
    }

    std::string toString() const
    {
        return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(width) + ", " + std::to_string(height) + ")");
    }

    bool operator==(const Rect<T>& other)
    { return (x == other.x && y == other.y && width == other.width && height == other.height); }

    bool operator!=(const Rect<T>& other)
    { return !(operator==(other)); }
};

using Rectf = Rect<float>;
using Recti = Rect<int>;