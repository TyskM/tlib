
#pragma once
#include <cstdint>

template <typename T>
struct Vector3;
using Vector3f = Vector3<float>;

struct ColorRGBAf;
struct ColorHSVf;

// Represents a RGB color with values 0f-1f
struct ColorRGBf
{
    float r = 0;
    float g = 0;
    float b = 0;

    constexpr ColorRGBf() = default;
    constexpr ColorRGBf(float r, float g, float b) : r{ r }, g{ g }, b{ b } { }

    explicit ColorRGBf(uint8_t r, uint8_t g, uint8_t b);

    // This will strip the alpha component
    explicit ColorRGBf(const ColorRGBAf& rgba);

    // This will convert to HSV automatically
    explicit ColorRGBf(const ColorHSVf& hsv);

    // Implicit, there is no data loss.
    operator Vector3f() const;

    bool operator==(const ColorRGBf& other) { return r == other.r && g == other.g && b == other.b; }
    bool operator!=(const ColorRGBf& other) { return !(operator==(other)); }
};
