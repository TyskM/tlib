
#pragma once

#include <TLib/String.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Math.hpp>
#include <cstdint>
#include <numeric>

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

    Vector2<T> midpoint(const Vector2<T>& other) const
    { return Vector2<T>(std::midpoint(x, other.x), std::midpoint(y, other.y)); }

    // In radians
    inline T angle() const
    { return atan2(y, x); }

    // In radians
    // X axis, 180 counter clockwise
    [[deprecated]]
    T angleTo(const Vector2<T>& other) const
    { return (other - *this).angle(); }

    // In radians
    T angleTo360YAxisCW(const Vector2<T>& other) const
    {
        auto diff = other - *this;
        T a = atan2(diff.x, diff.y);
        a = fmod(a + math::deg2rad(360.f), math::deg2rad(360.f)); 
        return a;
    }

    T angleTo180YAxisCW(const Vector2<T>& other) const
    {
        auto diff = other - *this;
        T a = atan2(diff.x, diff.y);
        return a;
    }

    // This is what Chipmunk physics uses
    T angleTo180YAxisCCW(const Vector2<T>& other) const
    {
        auto diff = other - *this;
        T a = -atan2(diff.x, diff.y);
        return a;
    }

    void rotate(T radians)
    { *this = rotated(radians); }

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
        if (len  == T(0)) { return rv;   }
        if (rv.x != T(0)) { rv.x /= len; }
        if (rv.y != T(0)) { rv.y /= len; }
        return rv;
    }

    T dot(const Vector2<T>& other) const
    { return x * other.x + y * other.y; }

    T cross(const Vector2<T>& other) const
    { return x * other.y - y * other.x; }

    Vector2<T> reflect(const Vector2<T>& normal) const
    { return (normal * 2) * dot(normal) - *this; }

    Vector2<T> sign() const
    { return Vector2<T>(math::sign(x), math::sign(y)); }

    T length() const
    { return static_cast<T>(std::sqrt(x * x + y * y)); }

    T lengthSquared() const
    { return x * x + y * y; }

    T distanceTo(const Vector2<T>& other) const
    { return static_cast<T>(std::sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y))); }

    T distanceToSquared(const Vector2<T>& other) const
    { return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y); }

    // Distance to the city of Manhattan https://en.wikipedia.org/wiki/Manhattan
    // jk https://simple.wikipedia.org/wiki/Manhattan_distance
    T distanceToManhattan(const Vector2<T>& other) const
    { return static_cast<T>(std::abs(x - other.x)) + static_cast<T>(std::abs(y - other.y)); }

    Vector2<T> relMultiply(const Vector2<T> other, T amount)
    { return (*this - other) * amount + other; }

    [[nodiscard]]
    bool isNan() const
    { return std::isnan(x) || std::isnan(y); }

    [[nodiscard]]
    bool isInf() const
    { return std::isinf(x) || std::isinf(y); }

    [[nodiscard]]
    bool isSane() const
    { return !isNan() && !isInf(); }

    Vector2<T> floored()    const { return Vector2<T>( static_cast<T>(std::floor(x)),      static_cast<T>(std::floor(y))      ); }
    Vector2<T> ceiled()     const { return Vector2<T>( static_cast<T>(std::ceil(x)),       static_cast<T>(std::ceil(y))       ); }
    Vector2<T> rounded()    const { return Vector2<T>( static_cast<T>(std::round(x)),      static_cast<T>(std::round(y))      ); }
    Vector2<T> abs()        const { return Vector2<T>( static_cast<T>(std::abs(x)),        static_cast<T>(std::abs(y))        ); }
    Vector2<T> sqrt()       const { return Vector2<T>( static_cast<T>(std::sqrt(x)),       static_cast<T>(std::sqrt(y))       ); }
    Vector2<T> pow(T value) const { return Vector2<T>( static_cast<T>(std::pow(x, value)), static_cast<T>(std::pow(y, value)) ); }

    Vector2<T> mod(T value) const
    {
        if constexpr (std::is_floating_point<T>())
        { return Vector2<T>(fmod(x, value), fmod(y, value)); }
        else
        { return Vector2<T>(x % value, y % value); }
    }

    Vector2<T> lerp(Vector2<T> target, T delta) const
    {
        return Vector2<T>(
        static_cast<T>(std::lerp(x, target.x, delta)),
        static_cast<T>(std::lerp(y, target.y, delta)) );
    }

    String toString() const { return fmt::format("({}, {})", x, y); }
    operator String() const { return toString(); }
    operator bool()   const { return x || y; }

    bool       operator==(const Vector2<T>& other)   const { return x == other.x && y == other.y; }
    bool       operator!=(const Vector2<T>& other)   const { return !(operator==(other)); }
    Vector2<T> operator- ()                          const { return Vector2<T>(-x, -y); }

    Vector2<T> operator+ (const Vector2<T>& v) const { return Vector2<T>(x + v.x, y + v.y); }
    Vector2<T> operator- (const Vector2<T>& v) const { return Vector2<T>(x - v.x, y - v.y); }
    Vector2<T> operator* (const Vector2<T>& v) const { return Vector2<T>(x * v.x, y * v.y); }
    Vector2<T> operator/ (const Vector2<T>& v) const { return Vector2<T>(x / v.x, y / v.y); }
    Vector2<T> operator+=(const Vector2<T>& v)       { x += v.x, y += v.y; return *this; }
    Vector2<T> operator-=(const Vector2<T>& v)       { x -= v.x, y -= v.y; return *this; }
    Vector2<T> operator*=(const Vector2<T>& v)       { x *= v.x, y *= v.y; return *this; }
    Vector2<T> operator/=(const Vector2<T>& v)       { x /= v.x, y /= v.y; return *this; }

    template <typename T> Vector2<T> operator+ (const T& v) const { return Vector2<T>(x + v, y + v); }
    template <typename T> Vector2<T> operator- (const T& v) const { return Vector2<T>(x - v, y - v); }
    template <typename T> Vector2<T> operator* (const T& v) const { return Vector2<T>(x * v, y * v); }
    template <typename T> Vector2<T> operator/ (const T& v) const { return Vector2<T>(x / v, y / v); }
    template <typename T> Vector2<T> operator+=(const T& v)       { x += v; y += v; return *this; }
    template <typename T> Vector2<T> operator-=(const T& v)       { x -= v; y -= v; return *this; }
    template <typename T> Vector2<T> operator*=(const T& v)       { x *= v; y *= v; return *this; }
    template <typename T> Vector2<T> operator/=(const T& v)       { x /= v; y /= v; return *this; }
};
using Vector2f  = Vector2<float>;
using Vector2i  = Vector2<int32_t>;
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

namespace eastl
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
