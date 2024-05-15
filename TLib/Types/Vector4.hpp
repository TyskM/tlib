
#pragma once

#include <TLib/String.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Math.hpp>
#include <glm/vec4.hpp>

template <typename T>
struct Vector4
{
    using value_type = T;

    constexpr Vector4() = default;
    constexpr Vector4(T xv, T yv, T zv, T wv) : x{ xv }, y{ yv }, z{ zv }, w{ wv } { }
    constexpr Vector4(const glm::vec<4, T>& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } { }

    glm::vec<4, T> toGlm() const { return glm::vec<4, T>(x, y, z, w); }

    T x = 0;
    T y = 0;
    T z = 0;
    T w = 0;

    Vector4<T> floored()    const { return Vector4<T>( floor(x),     floor(y),     floor(z),     floor(w)     ); }
    Vector4<T> ceiled()     const { return Vector4<T>( ceil(x),      ceil(y),      ceil(z),      ceil(w)      ); }
    Vector4<T> rounded()    const { return Vector4<T>( round(x),     round(y),     round(z),     round(w)     ); }
    Vector4<T> abs()        const { return Vector4<T>( std::abs(x),  std::abs(y),  std::abs(z),  std::abs(w)  ); }
    Vector4<T> sqrt()       const { return Vector4<T>( std::sqrt(x), std::sqrt(y), std::sqrt(z), std::sqrt(w) ); }

    Vector4<T> pow(T value) const
    {
        return Vector4<T>(std::pow(x, value),
                          std::pow(y, value),
                          std::pow(z, value),
                          std::pow(w, value));
    }
    
    Vector4<T> stepify(T step) const
    {
        return Vector4<T>(math::stepify(x, step),
                          math::stepify(y, step),
                          math::stepify(z, step),
                          math::stepify(w, step));
    }

    String toString() const { return fmt::format("({}, {}, {}, {})", x, y, z, w); }
    operator String() const { return toString(); }

    Vector4<T> operator+ (const Vector4<T>& v) const { return Vector4<T>(x+v.x, y+v.y, z+v.z, w+v.w); }
    Vector4<T> operator- (const Vector4<T>& v) const { return Vector4<T>(x-v.x, y-v.y, z-v.z, w-v.w); }
    Vector4<T> operator* (const Vector4<T>& v) const { return Vector4<T>(x*v.x, y*v.y, z*v.z, w*v.w); }
    Vector4<T> operator/ (const Vector4<T>& v) const { return Vector4<T>(x/v.x, y/v.y, z/v.z, w/v.w); }
    Vector4<T> operator+=(const Vector4<T>& v)       { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; }
    Vector4<T> operator-=(const Vector4<T>& v)       { x-=v.x; y-=v.y; z-=v.z; w-=v.w; return *this; }
    Vector4<T> operator*=(const Vector4<T>& v)       { x*=v.x; y*=v.y; z*=v.z; w*=v.w; return *this; }
    Vector4<T> operator/=(const Vector4<T>& v)       { x/=v.x; y/=v.y; z/=v.z; w/=v.w; return *this; }

    Vector4<T> operator* (const T v) const { return Vector4<T>(x*v, y*v, z*v, w*v); }
    Vector4<T> operator/ (const T v) const { return Vector4<T>(x/v, y/v, z/v, w/v); }
    Vector4<T> operator*=(const T v)       { x*=v; y*=v; z*=v; w*=v; return *this; }
    Vector4<T> operator/=(const T v)       { x/=v; y/=v; z/=v; w/=v; return *this; }
};

using Vector4f = Vector4<float>;
using Vector4i = Vector4<int>;
