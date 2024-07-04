
#pragma once

#include <TLib/String.hpp>
#include <TLib/Logging.hpp>
#include <glm/vec3.hpp>

template <typename T>
struct Vector3
{
    constexpr Vector3(T xv, T yv, T zv) : x{ xv }, y{ yv }, z{ zv } { }
    constexpr Vector3(T v)              : x{ v  }, y{ v  }, z{ v  } { }
    constexpr Vector3() { }

    template <typename CT>
    constexpr explicit Vector3(const CT& other)
    {
        x = static_cast<T>(other.x);
        y = static_cast<T>(other.y);
        z = static_cast<T>(other.z);
    }

    glm::vec<3, T> toGlm() const { return glm::vec<3, T>(x, y, z); }

    T x = 0;
    T y = 0;
    T z = 0;

    static Vector3<T> up()       { return Vector3<T>(0,  1,  0); }
    static Vector3<T> down()     { return -up();                 }
    static Vector3<T> right()    { return Vector3<T>(1,  0,  0); }
    static Vector3<T> left()     { return -right();              }
    static Vector3<T> forward()  { return Vector3<T>(0,  0, -1); }
    static Vector3<T> backward() { return -forward();            }

    Vector3<T> floored()    const { return { floor(x), floor(y), floor(z) }; }
    Vector3<T> ceiled()     const { return { ceil(x),  ceil(y),  ceil(z)  }; }
    Vector3<T> rounded()    const { return { round(x), round(y), round(z) }; }
    Vector3<T> abs()        const { return { std::abs(x),  std::abs(y),  std::abs(z)  }; }
    Vector3<T> sqrt()       const { return { std::sqrt(x), std::sqrt(y), std::sqrt(z) }; }
    Vector3<T> pow(T value) const { return { std::pow(x, value), std::pow(y, value), std::pow(z, value) }; }

    T length() const
    { return static_cast<T>(std::sqrt(x * x + y * y + z * z)); }

    T lengthSquared() const
    { return x * x + y * y + z * z; }

    T distanceTo(const Vector3<T>& other) const
    {
        return static_cast<T>(std::sqrt(
            (x - other.x) * (x - other.x) +
            (y - other.y) * (y - other.y) +
            (z - other.z) * (z - other.z))); }

    T distanceToSquared(const Vector3<T>& other) const
    {
        return (x - other.x) * (x - other.x) +
               (y - other.y) * (y - other.y) +
               (z - other.z) * (z - other.z);
    }

    void normalize()
    {
        *this = normalized();
    }

    Vector3<T> normalized() const
    {
        Vector3<T> rv = *this;
        T len = length();
        if (rv.x != 0) rv.x /= len;
        if (rv.y != 0) rv.y /= len;
        if (rv.z != 0) rv.z /= len;
        return rv;
    }

    Vector3<T> cross(Vector3<T> other) const
    {
        return Vector3<T>( (y * other.z) - (z * other.y),
                           (z * other.x) - (x * other.z),
                           (x * other.y) - (y * other.x)  );
    }

    float dot(Vector3<T> other) const
    { return x * other.x + y * other.y + z * other.z; }

    String toString() const { return fmt::format("({}, {}, {})", x, y, z); }
    operator String() const { return toString(); }

    bool       operator==(const Vector3<T>   v) const { return x == v.x && y == v.y && z == v.z; }
    bool       operator!=(const Vector3<T>   v) const { return !(operator==(v)); }
    Vector3<T> operator- ()                     const { return Vector3<T>(-x, -y, -z); }
    Vector3<T> operator+ (const Vector3<T>&  v) const { return Vector3<T>(x + v.x, y + v.y, z + v.z); }
    Vector3<T> operator- (const Vector3<T>&  v) const { return Vector3<T>(x - v.x, y - v.y, z - v.z); }
    Vector3<T> operator* (const Vector3<T>&  v) const { return Vector3<T>(x * v.x, y * v.y, z * v.z); }
    Vector3<T> operator/ (const Vector3<T>&  v) const { return Vector3<T>(x / v.x, y / v.y, z / v.z); }
    Vector3<T> operator* (const int          v) const { return Vector3<T>(x * v, y * v, z * v); }
    Vector3<T> operator/ (const int          v) const { return Vector3<T>(x / v, y / v, z / v); }
    Vector3<T> operator* (const float        v) const { return Vector3<T>(x * v, y * v, z * v); }
    Vector3<T> operator/ (const float        v) const { return Vector3<T>(x / v, y / v, z / v); }
    Vector3<T> operator+=(const Vector3<T>&  v)       { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3<T> operator-=(const Vector3<T>&  v)       { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector3<T> operator*=(const Vector3<T>&  v)       { x *= v.x; y *= v.y; z *= v.z; return *this; }
    Vector3<T> operator/=(const Vector3<T>&  v)       { x /= v.x; y /= v.y; z /= v.z; return *this; }
    Vector3<T> operator*=(const int          v)       { x *=   v; y *=   v; z *=   v; return *this; }
    Vector3<T> operator/=(const int          v)       { x /=   v; y /=   v; z /=   v; return *this; }
    Vector3<T> operator*=(const float        v)       { x *=   v; y *=   v; z *=   v; return *this; }
    Vector3<T> operator/=(const float        v)       { x /=   v; y /=   v; z /=   v; return *this; }



};

using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;
