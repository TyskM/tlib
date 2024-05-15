
#pragma once

#include <TLib/Types/Vector3.hpp>
#include <TLib/Types/ColorHSVf.hpp>
#include <cstdint>

// Represents a RGB color with values 0f-1f
struct ColorRGBf
{
    float r = 0;
    float g = 0;
    float b = 0;

    constexpr ColorRGBf() = default;
    constexpr ColorRGBf(float r, float g, float b) : r{ r }, g{ g }, b{ b } { }

    constexpr ColorRGBf(uint8_t r, uint8_t g, uint8_t b) :
                     r{ uint8ToFloat(r) },
                     g{ uint8ToFloat(g) },
                     b{ uint8ToFloat(b) } { }

    /// HSV Conversions
    constexpr ColorRGBf(const ColorHSVf& hsv)
    {
        auto ret = hsvToRgb(hsv.h, hsv.s, hsv.v);
        r = ret[0]; g = ret[1]; b = ret[2];
    }
    constexpr ColorHSVf toHsv() const
    {
        auto ret = rgbToHsv(r, g, b);
        return ColorHSVf(ret[0], ret[1], ret[2]);
    }
    operator ColorHSVf() const { return toHsv(); }

    // Implicit, there is no data loss.
    constexpr Vector3f toVec3()   const { return Vector3f(r, g, b); }
    constexpr operator Vector3f() const { return toVec3(); }

    bool operator==(const ColorRGBf& other) { return r == other.r && g == other.g && b == other.b; }
    bool operator!=(const ColorRGBf& other) { return !(operator==(other)); }
};
