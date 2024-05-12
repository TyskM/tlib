#pragma once

#include <string>
#include <cmath>
#include <limits>
#include <TLib/String.hpp>
#include <TLib/Math.hpp>
#include <TLib/Macros.hpp>
#include <TLib/EASTL.hpp>
#include <TLib/Logging.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

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

    Vector2<T> midpoint(const Vector2<T>& other) const
    { return Vector2<T>(std::midpoint(x, other.x), std::midpoint(y, other.y)); }

    // In radians
    inline T angle() const
    { return atan2(y, x); }

    // In radians
    T angleTo(const Vector2<T>& other) const
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

    // Distance to the city of Manhattan https://en.wikipedia.org/wiki/Manhattan
    // https://simple.wikipedia.org/wiki/Manhattan_distance
    T distanceToManhattan(const Vector2<T>& other) const
    { return static_cast<T>(std::abs(x - other.x)) + static_cast<T>(std::abs(y - other.y)); }

    Vector2<T> relMultiply(const Vector2<T> other, T amount)
    { return (*this - other) * amount + other; }

    [[nodiscard]]
    bool isNan() const
    { return std::isnan(x) || std::isnan(y); }

    String toString() const
    { return this->operator String(); }

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

    operator   String()                             const { return String("(" + std::to_string(x) + ", " + std::to_string(y) + ")"); }
    bool       operator==(const Vector2<T>& other)  const { return x == other.x && y == other.y; }
    bool       operator!=(const Vector2<T>& other)  const { return !(operator==(other)); }
    Vector2<T> operator-()                          const { return Vector2<T>(-x, -y); }
    Vector2<T> operator+(const Vector2<T>& other)   const { return Vector2<T>(x + other.x, y + other.y); }
    Vector2<T> operator-(const Vector2<T>& other)   const { return Vector2<T>(x - other.x, y - other.y); }
    Vector2<T> operator*(const Vector2<T>& other)   const { return Vector2<T>(x * other.x, y * other.y); }
    Vector2<T> operator/(const Vector2<T>& other)   const { return Vector2<T>(x / other.x, y / other.y); }
    Vector2<T> operator+(const int   num)           const { return Vector2<T>(x + num, y + num); }
    Vector2<T> operator-(const int   num)           const { return Vector2<T>(x - num, y - num); }
    Vector2<T> operator*(const int   num)           const { return Vector2<T>(x * num, y * num); }
    Vector2<T> operator/(const int   num)           const { return Vector2<T>(x / num, y / num); }
    Vector2<T> operator+(const float num)           const { return Vector2<T>(x + num, y + num); }
    Vector2<T> operator-(const float num)           const { return Vector2<T>(x - num, y - num); }
    Vector2<T> operator*(const float num)           const { return Vector2<T>(x * num, y * num); }
    Vector2<T> operator/(const float num)           const { return Vector2<T>(x / num, y / num); }
    Vector2<T> operator+=(const Vector2<T>& other)        { return Vector2<T>(x += other.x, y += other.y); }
    Vector2<T> operator-=(const Vector2<T>& other)        { return Vector2<T>(x -= other.x, y -= other.y); }
    Vector2<T> operator*=(const Vector2<T>& other)        { return Vector2<T>(x *= other.x, y *= other.y); }
    Vector2<T> operator/=(const Vector2<T>& other)        { return Vector2<T>(x /= other.x, y /= other.y); }
    Vector2<T> operator*=(const int num)                  { return Vector2<T>(x *= num, y *= num); }
    Vector2<T> operator/=(const int num)                  { return Vector2<T>(x /= num, y /= num); }
    Vector2<T> operator*=(const float num)                { return Vector2<T>(x *= num, y *= num); }
    Vector2<T> operator/=(const float num)                { return Vector2<T>(x /= num, y /= num); }

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

    std::string toString() const
    { return this->operator std::string(); }

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

    constexpr Vector4() = default;
    constexpr Vector4(T xv, T yv, T zv, T wv) : x{ xv }, y{ yv }, z{ zv }, w{ wv } { }
    constexpr Vector4(const glm::vec<4, T>& v) : x{ v.x }, y{ v.y }, z{ v.z }, w{ v.w } { }

    glm::vec<4, T> toGlm() const { return glm::vec<4, T>(x, y, z, w); }

    T x = 0;
    T y = 0;
    T z = 0;
    T w = 0;

    Vector4<T> floored()    const { return Vector4<T>( floor(x), floor(y), floor(z), floor(w)                                         ); }
    Vector4<T> ceiled()     const { return Vector4<T>( ceil(x), ceil(y), ceil(z), ceil(w)                                             ); }
    Vector4<T> rounded()    const { return Vector4<T>( round(x), round(y), round(z), round(w)                                         ); }
    Vector4<T> abs()        const { return Vector4<T>( std::abs(x), std::abs(y), std::abs(z), std::abs(w)                             ); }
    Vector4<T> sqrt()       const { return Vector4<T>( std::sqrt(x), std::sqrt(y), std::sqrt(z), std::sqrt(w)                         ); }
    Vector4<T> pow(T value) const { return Vector4<T>( std::pow(x, value), std::pow(y, value), std::pow(z, value), std::pow(w, value) ); }
    
    Vector4<T> stepify(T step) const
    {
        return Vector4<T>(math::stepify(x, step),
                          math::stepify(y, step),
                          math::stepify(z, step),
                          math::stepify(w, step));
    }

    Vector4<T> operator+ (const Vector4<T>& v) const { return Vector4<T>(x+v.x, y+v.y, z+v.z, w+v.w); }
    Vector4<T> operator+=(const Vector4<T>& v) const { return Vector4<T>(x+v.x, y+v.y, z+v.z, w+v.w); }
    Vector4<T> operator- (const Vector4<T>& v) const { return Vector4<T>(x-v.x, y-v.y, z-v.z, w-v.w); }
    Vector4<T> operator-=(const Vector4<T>& v) const { return Vector4<T>(x-v.x, y-v.y, z-v.z, w-v.w); }
    Vector4<T> operator* (const Vector4<T>& v) const { return Vector4<T>(x*v.x, y*v.y, z*v.z, w*v.w); }
    Vector4<T> operator*=(const Vector4<T>& v) const { return Vector4<T>(x*v.x, y*v.y, z*v.z, w*v.w); }
    Vector4<T> operator/ (const Vector4<T>& v) const { return Vector4<T>(x/v.x, y/v.y, z/v.z, w/v.w); }
    Vector4<T> operator/=(const Vector4<T>& v) const { return Vector4<T>(x/v.x, y/v.y, z/v.z, w/v.w); }

    Vector4<T> operator* (const T v) const { return Vector4<T>(x*v, y*v, z*v, w*v); }
    Vector4<T> operator*=(const T v) const { return Vector4<T>(x*v, y*v, z*v, w*v); }
    Vector4<T> operator/ (const T v) const { return Vector4<T>(x/v, y/v, z/v, w/v); }
    Vector4<T> operator/=(const T v) const { return Vector4<T>(x/v, y/v, z/v, w/v); }
};

using Vector4f = Vector4<float>;

// Represents a RGBA color with values 0f-1f
struct ColorRGBAf
{
    constexpr ColorRGBAf() = default;

    ColorRGBAf(String hex)
    {
        if (hex.at(0) == '#') { hex.erase(0, 1); }
        ASSERT( hex.size() == (6) ); // Invalid Hex code

        int ir, ig, ib;
        sscanf(hex.c_str(), "%02x%02x%02x", &ir, &ig, &ib);
        r = ir/255.f;
        g = ig/255.f;
        b = ib/255.f;
    }

    ColorRGBAf(const char* hex) : ColorRGBAf(String(hex)) { }

    constexpr ColorRGBAf(float rv, float gv, float bv, float av = 1.f) :
        r{ rv }, g{ gv }, b{ bv }, a{ av } { }

    explicit constexpr ColorRGBAf(int rv, int gv, int bv, int av = 255) :
        r{ uint8ToFloat(rv) }, g{ uint8ToFloat(gv) }, b{ uint8ToFloat(bv) }, a{ uint8ToFloat(av) } { }

    operator Vector4f() const
    { return Vector4f(r, g, b, a); }

    ColorRGBAf& setR(float r) { this->r = r; return *this; }
    ColorRGBAf& setG(float g) { this->g = g; return *this; }
    ColorRGBAf& setB(float b) { this->b = b; return *this; }
    ColorRGBAf& setA(float a) { this->a = a; return *this; }

    String toString() const { return fmt::format("({}, {}, {}, {})", r, g, b, a); }

    float r = 0;
    float g = 0;
    float b = 0;
    float a = 1.f;

    bool operator==(const ColorRGBAf& other) { return r == other.r && g == other.g && b == other.b && a == other.a; }
    bool operator!=(const ColorRGBAf& other) { return !(operator==(other)); }

    static constexpr inline ColorRGBAf red()         { return ColorRGBAf{ 255, 0,   0    }; }
    static constexpr inline ColorRGBAf green()       { return ColorRGBAf{ 0,   255, 0    }; }
    static constexpr inline ColorRGBAf blue()        { return ColorRGBAf{ 0,   0,   255  }; }
    static constexpr inline ColorRGBAf white()       { return ColorRGBAf{ 255, 255, 255  }; }
    static constexpr inline ColorRGBAf black()       { return ColorRGBAf{ 0,   0,   0    }; }
    static constexpr inline ColorRGBAf transparent() { return ColorRGBAf{ 0,   0,   0, 0 }; }
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

    static constexpr inline ColorRGBAf maroon                   () { return ColorRGBAf(128,0,0)     ; };
    static constexpr inline ColorRGBAf darkRed                  () { return ColorRGBAf(139,0,0)     ; };
    static constexpr inline ColorRGBAf brown                    () { return ColorRGBAf(165,42,42)   ; };
    static constexpr inline ColorRGBAf firebrick                () { return ColorRGBAf(178,34,34)   ; };
    static constexpr inline ColorRGBAf crimson                  () { return ColorRGBAf(220,20,60)   ; };
    static constexpr inline ColorRGBAf tomato                   () { return ColorRGBAf(255,99,71)   ; };
    static constexpr inline ColorRGBAf coral                    () { return ColorRGBAf(255,127,80)  ; };
    static constexpr inline ColorRGBAf indianRed                () { return ColorRGBAf(205,92,92)   ; };
    static constexpr inline ColorRGBAf lightCoral               () { return ColorRGBAf(240,128,128) ; };
    static constexpr inline ColorRGBAf darkSalmon               () { return ColorRGBAf(233,150,122) ; };
    static constexpr inline ColorRGBAf salmon                   () { return ColorRGBAf(250,128,114) ; };
    static constexpr inline ColorRGBAf lightSalmon              () { return ColorRGBAf(255,160,122) ; };
    static constexpr inline ColorRGBAf orangeRed                () { return ColorRGBAf(255,69,0)    ; };
    static constexpr inline ColorRGBAf darkOrange               () { return ColorRGBAf(255,140,0)   ; };
    static constexpr inline ColorRGBAf darkGoldenRod            () { return ColorRGBAf(184,134,11)  ; };
    static constexpr inline ColorRGBAf goldenRod                () { return ColorRGBAf(218,165,32)  ; };
    static constexpr inline ColorRGBAf paleGoldenRod            () { return ColorRGBAf(238,232,170) ; };
    static constexpr inline ColorRGBAf darkKhaki                () { return ColorRGBAf(189,183,107) ; };
    static constexpr inline ColorRGBAf khaki                    () { return ColorRGBAf(240,230,140) ; };
    static constexpr inline ColorRGBAf olive                    () { return ColorRGBAf(128,128,0)   ; };
    static constexpr inline ColorRGBAf yellowGreen              () { return ColorRGBAf(154,205,50)  ; };
    static constexpr inline ColorRGBAf darkOliveGreen           () { return ColorRGBAf(85,107,47)   ; };
    static constexpr inline ColorRGBAf oliveDrab                () { return ColorRGBAf(107,142,35)  ; };
    static constexpr inline ColorRGBAf lawnGreen                () { return ColorRGBAf(124,252,0)   ; };
    static constexpr inline ColorRGBAf chartreuse               () { return ColorRGBAf(127,255,0)   ; };
    static constexpr inline ColorRGBAf greenYellow              () { return ColorRGBAf(173,255,47)  ; };
    static constexpr inline ColorRGBAf darkGreen                () { return ColorRGBAf(0,100,0)     ; };
    static constexpr inline ColorRGBAf forestGreen              () { return ColorRGBAf(34,139,34)   ; };
    static constexpr inline ColorRGBAf limeGreen                () { return ColorRGBAf(50,205,50)   ; };
    static constexpr inline ColorRGBAf lightGreen               () { return ColorRGBAf(144,238,144) ; };
    static constexpr inline ColorRGBAf paleGreen                () { return ColorRGBAf(152,251,152) ; };
    static constexpr inline ColorRGBAf darkSeaGreen             () { return ColorRGBAf(143,188,143) ; };
    static constexpr inline ColorRGBAf mediumSpringGreen        () { return ColorRGBAf(0,250,154)   ; };
    static constexpr inline ColorRGBAf springGreen              () { return ColorRGBAf(0,255,127)   ; };
    static constexpr inline ColorRGBAf seaGreen                 () { return ColorRGBAf(46,139,87)   ; };
    static constexpr inline ColorRGBAf mediumAquaMarine         () { return ColorRGBAf(102,205,170) ; };
    static constexpr inline ColorRGBAf mediumSeaGreen           () { return ColorRGBAf(60,179,113)  ; };
    static constexpr inline ColorRGBAf lightSeaGreen            () { return ColorRGBAf(32,178,170)  ; };
    static constexpr inline ColorRGBAf darkSlateGray            () { return ColorRGBAf(47,79,79)    ; };
    static constexpr inline ColorRGBAf teal                     () { return ColorRGBAf(0,128,128)   ; };
    static constexpr inline ColorRGBAf darkCyan                 () { return ColorRGBAf(0,139,139)   ; };
    static constexpr inline ColorRGBAf aqua                     () { return ColorRGBAf(0,255,255)   ; };
    static constexpr inline ColorRGBAf lightCyan                () { return ColorRGBAf(224,255,255) ; };
    static constexpr inline ColorRGBAf darkTurquoise            () { return ColorRGBAf(0,206,209)   ; };
    static constexpr inline ColorRGBAf turquoise                () { return ColorRGBAf(64,224,208)  ; };
    static constexpr inline ColorRGBAf mediumTurquoise          () { return ColorRGBAf(72,209,204)  ; };
    static constexpr inline ColorRGBAf paleTurquoise            () { return ColorRGBAf(175,238,238) ; };
    static constexpr inline ColorRGBAf aquaMarine               () { return ColorRGBAf(127,255,212) ; };
    static constexpr inline ColorRGBAf powderBlue               () { return ColorRGBAf(176,224,230) ; };
    static constexpr inline ColorRGBAf cadetBlue                () { return ColorRGBAf(95,158,160)  ; };
    static constexpr inline ColorRGBAf cornFlowerBlue           () { return ColorRGBAf(100,149,237) ; };
    static constexpr inline ColorRGBAf deepSkyBlue              () { return ColorRGBAf(0,191,255)   ; };
    static constexpr inline ColorRGBAf dodgerBlue               () { return ColorRGBAf(30,144,255)  ; };
    static constexpr inline ColorRGBAf lightBlue                () { return ColorRGBAf(173,216,230) ; };
    static constexpr inline ColorRGBAf skyBlue                  () { return ColorRGBAf(135,206,235) ; };
    static constexpr inline ColorRGBAf lightSkyBlue             () { return ColorRGBAf(135,206,250) ; };
    static constexpr inline ColorRGBAf midnightBlue             () { return ColorRGBAf(25,25,112)   ; };
    static constexpr inline ColorRGBAf navy                     () { return ColorRGBAf(0,0,128)     ; };
    static constexpr inline ColorRGBAf darkBlue                 () { return ColorRGBAf(0,0,139)     ; };
    static constexpr inline ColorRGBAf mediumBlue               () { return ColorRGBAf(0,0,205)     ; };
    static constexpr inline ColorRGBAf blueViolet               () { return ColorRGBAf(138,43,226)  ; };
    static constexpr inline ColorRGBAf indigo                   () { return ColorRGBAf(75,0,130)    ; };
    static constexpr inline ColorRGBAf darkSlateBlue            () { return ColorRGBAf(72,61,139)   ; };
    static constexpr inline ColorRGBAf slateBlue                () { return ColorRGBAf(106,90,205)  ; };
    static constexpr inline ColorRGBAf mediumSlateBlue          () { return ColorRGBAf(123,104,238) ; };
    static constexpr inline ColorRGBAf mediumPurple             () { return ColorRGBAf(147,112,219) ; };
    static constexpr inline ColorRGBAf darkViolet               () { return ColorRGBAf(148,0,211)   ; };
    static constexpr inline ColorRGBAf darkOrchid               () { return ColorRGBAf(153,50,204)  ; };
    static constexpr inline ColorRGBAf mediumOrchid             () { return ColorRGBAf(186,85,211)  ; };
    static constexpr inline ColorRGBAf thistle                  () { return ColorRGBAf(216,191,216) ; };
    static constexpr inline ColorRGBAf plum                     () { return ColorRGBAf(221,160,221) ; };
    static constexpr inline ColorRGBAf violet                   () { return ColorRGBAf(238,130,238) ; };
    static constexpr inline ColorRGBAf orchid                   () { return ColorRGBAf(218,112,214) ; };
    static constexpr inline ColorRGBAf mediumVioletRed          () { return ColorRGBAf(199,21,133)  ; };
    static constexpr inline ColorRGBAf paleVioletRed            () { return ColorRGBAf(219,112,147) ; };
    static constexpr inline ColorRGBAf deepPink                 () { return ColorRGBAf(255,20,147)  ; };
    static constexpr inline ColorRGBAf hotPink                  () { return ColorRGBAf(255,105,180) ; };
    static constexpr inline ColorRGBAf lightPink                () { return ColorRGBAf(255,182,193) ; };
    static constexpr inline ColorRGBAf pink                     () { return ColorRGBAf(255,192,203) ; };
    static constexpr inline ColorRGBAf antiqueWhite             () { return ColorRGBAf(250,235,215) ; };
    static constexpr inline ColorRGBAf beige                    () { return ColorRGBAf(245,245,220) ; };
    static constexpr inline ColorRGBAf bisque                   () { return ColorRGBAf(255,228,196) ; };
    static constexpr inline ColorRGBAf blanchedAlmond           () { return ColorRGBAf(255,235,205) ; };
    static constexpr inline ColorRGBAf wheat                    () { return ColorRGBAf(245,222,179) ; };
    static constexpr inline ColorRGBAf cornSilk                 () { return ColorRGBAf(255,248,220) ; };
    static constexpr inline ColorRGBAf lemonChiffon             () { return ColorRGBAf(255,250,205) ; };
    static constexpr inline ColorRGBAf lightYellow              () { return ColorRGBAf(255,255,224) ; };
    static constexpr inline ColorRGBAf saddleBrown              () { return ColorRGBAf(139,69,19)   ; };
    static constexpr inline ColorRGBAf sienna                   () { return ColorRGBAf(160,82,45)   ; };
    static constexpr inline ColorRGBAf chocolate                () { return ColorRGBAf(210,105,30)  ; };
    static constexpr inline ColorRGBAf peru                     () { return ColorRGBAf(205,133,63)  ; };
    static constexpr inline ColorRGBAf sandyBrown               () { return ColorRGBAf(244,164,96)  ; };
    static constexpr inline ColorRGBAf burlyWood                () { return ColorRGBAf(222,184,135) ; };
    static constexpr inline ColorRGBAf tan                      () { return ColorRGBAf(210,180,140) ; };
    static constexpr inline ColorRGBAf rosyBrown                () { return ColorRGBAf(188,143,143) ; };
    static constexpr inline ColorRGBAf moccasin                 () { return ColorRGBAf(255,228,181) ; };
    static constexpr inline ColorRGBAf navajoWhite              () { return ColorRGBAf(255,222,173) ; };
    static constexpr inline ColorRGBAf peachPuff                () { return ColorRGBAf(255,218,185) ; };
    static constexpr inline ColorRGBAf mistyRose                () { return ColorRGBAf(255,228,225) ; };
    static constexpr inline ColorRGBAf lavenderBlush            () { return ColorRGBAf(255,240,245) ; };
    static constexpr inline ColorRGBAf linen                    () { return ColorRGBAf(250,240,230) ; };
    static constexpr inline ColorRGBAf oldLace                  () { return ColorRGBAf(253,245,230) ; };
    static constexpr inline ColorRGBAf papayaWhip               () { return ColorRGBAf(255,239,213) ; };
    static constexpr inline ColorRGBAf seaShell                 () { return ColorRGBAf(255,245,238) ; };
    static constexpr inline ColorRGBAf mintCream                () { return ColorRGBAf(245,255,250) ; };
    static constexpr inline ColorRGBAf slateGray                () { return ColorRGBAf(112,128,144) ; };
    static constexpr inline ColorRGBAf lightSlateGray           () { return ColorRGBAf(119,136,153) ; };
    static constexpr inline ColorRGBAf lightSteelBlue           () { return ColorRGBAf(176,196,222) ; };
    static constexpr inline ColorRGBAf lavender                 () { return ColorRGBAf(230,230,250) ; };
    static constexpr inline ColorRGBAf floralWhite              () { return ColorRGBAf(255,250,240) ; };
    static constexpr inline ColorRGBAf aliceBlue                () { return ColorRGBAf(240,248,255) ; };
    static constexpr inline ColorRGBAf ghostWhite               () { return ColorRGBAf(248,248,255) ; };
    static constexpr inline ColorRGBAf honeydew                 () { return ColorRGBAf(240,255,240) ; };
    static constexpr inline ColorRGBAf ivory                    () { return ColorRGBAf(255,255,240) ; };
    static constexpr inline ColorRGBAf azure                    () { return ColorRGBAf(240,255,255) ; };
    static constexpr inline ColorRGBAf snow                     () { return ColorRGBAf(255,250,250) ; };
    static constexpr inline ColorRGBAf dimGray                  () { return ColorRGBAf(105,105,105) ; };
    static constexpr inline ColorRGBAf gray                     () { return ColorRGBAf(128,128,128) ; };
    static constexpr inline ColorRGBAf darkGray                 () { return ColorRGBAf(169,169,169) ; };
    static constexpr inline ColorRGBAf silver                   () { return ColorRGBAf(192,192,192) ; };
    static constexpr inline ColorRGBAf lightGray                () { return ColorRGBAf(211,211,211) ; };
    static constexpr inline ColorRGBAf whiteSmoke               () { return ColorRGBAf(245,245,245) ; };
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

    operator Vector3f() const { return Vector3f(r, g, b); }

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

    Rect(const Vector2<T>& pos, const Vector2<T>& size) : x{ pos.x }, y{ pos.y }, width{ size.x }, height{ size.y } { }
    Rect(T x, T y, T width, T height) : x{ x }, y{ y }, width{ width }, height{ height } { }
    Rect(const Vector2<T>& pos, T width, T height) : x{pos.x}, y{pos.y}, width{width}, height{height} { }
    Rect(T x, T y, const Vector2<T>& size) : x{x}, y{y}, width{size.x}, height{size.y} { }
    Rect() = default;

    template <typename CT>
    constexpr explicit Rect(const CT& other)
    {
        x      = static_cast<T>(other.x);
        y      = static_cast<T>(other.y);
        width  = static_cast<T>(other.width);
        height = static_cast<T>(other.height);
    }

    static Rect fromLTRB(T left, T top, T right, T bottom)
    {
        if (right < left) { std::swap(right, left); }
        if (bottom < top) { std::swap(bottom, top); }
        return Rect<T>(left, top, right - left, bottom - top);
    }

    static Rect fromLTRB(const Vector2<T>& lt, const Vector2<T>& rb)
    { return fromLTRB(lt.x, lt.y, rb.x, rb.y); }

    Vector2<T> getPos() const
    { return Vector2<T>(x, y); }

    Vector2<T> getSize() const
    { return Vector2<T>(width, height); }

    void setPos(const Vector2<T>& v)
    { x = v.x; y = v.y; }

    void setSize(const Vector2<T>& v)
    { width = v.x; height = v.y; }

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

    Vector2<T> center() const
    { return getPos() + (getSize() / T(2)); }

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

struct Mat4f
{
private:
    glm::mat4 matrix;

public:
    Mat4f() = default;
    Mat4f(float x) { matrix = glm::mat4(x); }
    Mat4f(const glm::mat4& m) : matrix{ m } { }

    const float* data() const
    { return glm::value_ptr(matrix); }

    Mat4f translate(const Vector3f& transform) const
    { return glm::translate(matrix, glm::vec3(transform.x, transform.y, transform.z)); }

    Mat4f scale(const Vector3f& _scale) const
    { return glm::scale(matrix, glm::vec3(_scale.x, _scale.y, _scale.z)); }

    Mat4f scale(float x, float y, float z) const
    { return glm::scale(matrix, glm::vec3(x, y, z)); }

    Mat4f inverse() const
    { return glm::inverse(toGlm()); }

    glm::mat4 toGlm() const { return matrix; }

    Mat4f operator* (const Mat4f& other) { return matrix * other.matrix; }
    Mat4f operator*=(const Mat4f& other) { return matrix * other.matrix; }
};

struct Quat
{
    float w = 1.f;
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    Quat() = default;
    Quat(float w, float x, float y, float z) : w{ w }, x{ x }, y{ y }, z{ z } { }
    Quat(const glm::quat& q) : w{ q.w }, x{ q.x }, y{ q.y }, z{ q.z } { }
    Quat(float angle, const Vector3f& axis) { *this = angleAxis(angle, axis); }

    glm::quat toGlm() const { return glm::quat(w, x, y, z); }

    String toString() const { return fmt::format("({}, {}, {}, {})", w, x, y, z); }

    static Quat angleAxis(float angle, const Vector3f& axis)
    { return glm::angleAxis(angle, glm::vec3(axis.x, axis.y, axis.z)); }

    Vector3f forward() const
    { return inverse().rotated(Vector3f::forward()); }

    Vector3f right() const
    { return inverse().rotated(Vector3f::right()); }

    Vector3f up() const
    { return inverse().rotated(Vector3f::up()); }

    Quat inverse() const
    { return glm::inverse(toGlm()); }

    Vector3f rotated(const Vector3f& axis) const
    { return Vector3f(glm::rotate(toGlm(), glm::vec3{axis.x, axis.y, axis.z})); }

    Vector3f rotated(float axisx, float axisy, float axisz) const
    { return Vector3f(glm::rotate(toGlm(), glm::vec3{ axisx, axisy, axisz })); }

    Quat normalized() const
    { return glm::normalize(toGlm()); }

    Mat4f toMat4f() const
    { return glm::toMat4(toGlm()); }

    static Quat safeLookAt(const Vector3f& lookFrom,
                           const Vector3f& lookTo,
                           const Vector3f& up,
                           const Vector3f& alternativeUp)
    {
        // https://stackoverflow.com/questions/18172388/glm-quaternion-lookat-function
        Vector3f  direction       = lookTo - lookFrom;
        float     directionLength = direction.length();

        // Check if the direction is valid; Also deals with NaN
        if (!(directionLength > 0.0001))
            return glm::quat(1, 0, 0, 0); // Just return identity

        direction = direction.normalized();

        // Is the normal up (nearly) parallel to direction?
        if (abs(direction.dot(up)) > .9999f)
        { return glm::quatLookAt(direction.toGlm(), alternativeUp.toGlm()); }
        else
        { return glm::quatLookAt(direction.toGlm(), up.toGlm()); }
    }

    Quat operator* (const Quat& other) { return toGlm() * other.toGlm(); }
    Quat operator*=(const Quat& other) { return toGlm() * other.toGlm(); }
};

Mat4f    operator* (const Mat4f& a, const Mat4f& b)   { return a.toGlm() * b.toGlm(); }
Mat4f    operator*=(const Mat4f& a, const Mat4f& b)   { return a.toGlm() * b.toGlm(); }

Vector3f operator* (const Vector3f& v, const Quat& q) { return Vector3f(v.toGlm() * q.toGlm()); }
Vector3f operator*=(const Vector3f& v, const Quat& q) { return Vector3f(v.toGlm() * q.toGlm()); }

Vector3f operator* (const Vector3f& v, const Mat4f& m) { return Vector3f(glm::vec4(v.x, v.y, v.z, 1.f) * m.toGlm()); }
Vector3f operator*=(const Vector3f& v, const Mat4f& m) { return Vector3f(glm::vec4(v.x, v.y, v.z, 1.f) * m.toGlm()); }

Vector4f operator* (const Vector4f& v, const Mat4f& m) { return v.toGlm() * m.toGlm(); }
Vector4f operator*=(const Vector4f& v, const Mat4f& m) { return v.toGlm() * m.toGlm(); }
Vector4f operator* (const Mat4f& m, const Vector4f& v) { return m.toGlm() * v.toGlm(); }
Vector4f operator*=(const Mat4f& m, const Vector4f& v) { return m.toGlm() * v.toGlm(); }