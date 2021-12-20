#pragma once

#include <string>
#include <cmath>
#include "Math.hpp"

#ifdef BOX2D
    #include <box2d/box2d.h>
#endif
#ifdef SFML
    #include <SFML/Graphics.hpp>
#endif
#ifdef OPENGL
    #include <glm/vec3.hpp>
    #include <glm/mat4x4.hpp>
#endif


/// Vectors

struct Vector2f
{
    // https://github.com/godotengine/godot/blob/master/core/math/vector2.cpp

    constexpr Vector2f(float xv, float yv) : x{ xv }, y{ yv }            { }
    Vector2f() { }

    float x = 0;
    float y = 0;

    void rotate(float radians)
    { *this = rotated(radians); }

    // SIN AND COS USES RADIANS YOU DUMB IDIOT
    Vector2f rotated(const float& radians) const
    {
        float sinv = sin(radians);
        float cosv = cos(radians);
        return Vector2f(x * cosv - y * sinv, x * sinv + y * cosv);
    }

    void normalize()
    {
        float len = length();
        x == 0 ? 0 : x /= len;
        y == 0 ? 0 : y /= len;
    }

    Vector2f normalized()
    {
        Vector2f me = *this;
        me.normalize();
        return me;
    }

    float length() const
    { return sqrtf(x * x + y * y); }

    float lengthSquared() const
    { return x * x + y * y; }

    float dot(const Vector2f& other) const
    { return x * other.x + y * other.y; }

    float cross(const Vector2f& other) const
    { return x * other.y - y * other.x; }

    Vector2f reflect(const Vector2f& normal) const
    { return 2.f * normal * dot(normal) - *this; }

    std::string toString()
    { return this->operator std::string(); }

    Vector2f operator+(const Vector2f& other) const { return Vector2f(x + other.x, y + other.y); }
    Vector2f operator-(const Vector2f& other) const { return Vector2f(x - other.x, y - other.y); }
    Vector2f operator*(const Vector2f& other) const { return Vector2f(x * other.x, y * other.y); }
    Vector2f operator/(const Vector2f& other) const { return Vector2f(x / other.x, y / other.y); }
    Vector2f operator*(const int& num)        const { return Vector2f(x * num, y * num); }
    Vector2f operator/(const int& num)        const { return Vector2f(x / num, y / num); }
    Vector2f operator*(const float& num)      const { return Vector2f(x * num, y * num); }
    Vector2f operator/(const float& num)      const { return Vector2f(x / num, y / num); }
    Vector2f operator-()                      const { return Vector2f(-x, -y); }

    Vector2f operator+=(const Vector2f& other) { return Vector2f(x += other.x, y += other.y); }
    Vector2f operator-=(const Vector2f& other) { return Vector2f(x -= other.x, y -= other.y); }
    Vector2f operator*=(const Vector2f& other) { return Vector2f(x *= other.x, y *= other.y); }
    Vector2f operator/=(const Vector2f& other) { return Vector2f(x /= other.x, y /= other.y); }
    Vector2f operator*=(const int& num) { return Vector2f(x *= num, y *= num); }
    Vector2f operator/=(const int& num) { return Vector2f(x /= num, y /= num); }
    Vector2f operator*=(const float& num) { return Vector2f(x *= num, y *= num); }
    Vector2f operator/=(const float& num) { return Vector2f(x /= num, y /= num); }
    friend Vector2f operator* (float num, Vector2f v) { return v * num; }

    bool operator==(const Vector2f other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2f other) const { return !(operator==(other)); }

    operator std::string() const { return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ")"); }
        
#ifdef SFML
    Vector2f(sf::Vector2f other) : x{ other.x }, y{ other.y }  { }
    Vector2f(sf::Vector2i other) { x = other.x; y = other.y; }
    operator sf::Vector2f() const { return sf::Vector2f(x, y); }
#endif
#ifdef BOX2D
    Vector2f(b2Vec2 box2dVector2) : x{ box2dVector2.x }, y{ box2dVector2.y }  { }
    operator b2Vec2()       const { return b2Vec2(x, y); }
#endif
#ifdef GLM
    Vector2f(glm::vec2 other) : x{ other.x }, y{ other.y }  { }
    operator glm::vec2() { return glm::vec2(x, y); }
#endif
};

inline Vector2f floor(const Vector2f v) { return Vector2f(std::floor(v.x), std::floor(v.y)); }

struct Vector2i
{
    Vector2i(int xv, int yv) : x{ xv }, y{ yv } { }
    Vector2i(Vector2f v2f) : Vector2i(v2f.x, v2f.y) { }
        
    Vector2i() { }

    int x = 0;
    int y = 0;

    std::string toString()
    { return this->operator std::string(); }

    Vector2i operator+(const Vector2i& other) const { return Vector2i(x + other.x, y + other.y); }
    Vector2i operator-(const Vector2i& other) const { return Vector2i(x - other.x, y - other.y); }
    Vector2i operator*(const Vector2i& other) const { return Vector2i(x * other.x, y * other.y); }
    Vector2i operator/(const Vector2i& other) const { return Vector2i(x / other.x, y / other.y); }
    Vector2i operator*(const int& num)        const { return Vector2i(x * num, y * num); }
    Vector2i operator/(const int& num)        const { return Vector2i(x / num, y / num); }
    Vector2i operator*(const float& num)      const { return Vector2i(x * num, y * num); }
    Vector2i operator/(const float& num)      const { return Vector2i(x / num, y / num); }

    Vector2f operator+=(const Vector2i& other) { return Vector2i(x += other.x, y += other.y); }
    Vector2f operator-=(const Vector2i& other) { return Vector2i(x -= other.x, y -= other.y); }
    Vector2f operator*=(const Vector2i& other) { return Vector2i(x *= other.x, y *= other.y); }
    Vector2f operator/=(const Vector2i& other) { return Vector2i(x /= other.x, y /= other.y); }
    Vector2f operator*=(const int& num) { return Vector2i(x *= num, y *= num); }
    Vector2f operator/=(const int& num) { return Vector2i(x /= num, y /= num); }
    Vector2f operator*=(const float& num) { return Vector2i(x *= num, y *= num); }
    Vector2f operator/=(const float& num) { return Vector2i(x /= num, y /= num); }

    bool operator==(const Vector2i other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2i other) const { return !(operator==(other)); }


    operator Vector2f()     const { return Vector2f(x, y); }
    operator std::string()  const { return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ")"); }

#ifdef SFML
    Vector2i(sf::Vector2i sfv2i) : Vector2i(sfv2i.x, sfv2i.y) { }
    operator sf::Vector2i() const { return sf::Vector2i(x, y); }
    operator sf::Vector2f() const { return sf::Vector2f(x, y); }
#endif
};

inline Vector2i floor(const Vector2i v) { return v; }

struct Vector3f
{
    Vector3f(float xv, float yv, float zv) : x{xv}, y{yv}, z{zv} { }
    
    Vector3f() { }

    float x = 0;
    float y = 0;
    float z = 0;

    float length() const
    { return sqrtf(x * x + y * y + z * z); }

    Vector3f normalized()
    {
        float len = length();
        return Vector3f(x == 0 ? 0 : x /= len, y == 0 ? 0 : y /= len, z == 0 ? 0 : z /= len);
    }

    float dot(const Vector3f& other) const
    { return x * other.x + y * other.y + z * other.z; }

    Vector3f cross(const Vector3f& other) const
    {
        return Vector3f((y * other.z) - (z * other.y),
                        (z * other.x) - (x * other.z),
                        (x * other.y) - (y * other.x));
    }

    Vector3f abs() const
    { return Vector3f(fabs(x), fabs(y), fabs(z)); }

    Vector3f sign() const
    { return Vector3f(math::sign(x), math::sign(y), math::sign(z)); }

    std::string toString() { return this->operator std::string(); }

    Vector3f operator+(const Vector3f& other) const { return Vector3f(x + other.x, y + other.y, z + other.z); }
    Vector3f operator-(const Vector3f& other) const { return Vector3f(x - other.x, y - other.y, z - other.z); }
    Vector3f operator*(const Vector3f& other) const { return Vector3f(x * other.x, y * other.y, z * other.z); }
    Vector3f operator/(const Vector3f& other) const { return Vector3f(x / other.x, y / other.y, z / other.z); }
    Vector3f operator*(const float value)     const { return Vector3f(x * value, y * value, z * value); }
    Vector3f operator-()                      const { return Vector3f(-x, -y, -z); }
    bool     operator==(Vector3f other)       const { return x == other.x && y == other.y && z == other.z; }
    bool     operator!=(Vector3f other)       const { return !(operator==(other)); }
    operator std::string() const { return std::string("(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")"); }
    
#ifdef OPENGL
    Vector3f(glm::vec3 glVec) : x{glVec.x}, y{glVec.y}, z{glVec.z} { }
    operator glm::vec3() { return glm::vec3(x, y, z); }
#endif
};

/// Colors

float uint8ToFloat(uint8_t value)
{ return value / 255.f; }

uint8_t floatToUint8(float value)
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


/// Misc

#ifdef OPENGL
// A wrapper around glm::mat4 with convenience functions
struct Transform
{
    glm::mat4 matrix;

    Transform()                  { matrix = glm::mat4(1.f); }
    Transform(glm::mat4 matrixv) { matrix = matrixv; }

    Vector3f getTransform()               { return Vector3f(matrix[3][0], matrix[3][1], matrix[3][2]); }
    void     setTransform(Vector3f value) { matrix[3][0] = value.x; matrix[3][1] = value.y; matrix[3][2] = value.z; }

    glm::vec4 operator[](size_t index) { return matrix[index]; }
    operator glm::mat4() { return matrix; }
};
#endif