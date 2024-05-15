
#pragma once

#include <TLib/Types/ColorRGBi.hpp>

// Represents a RGBA color with values 0-255
struct ColorRGBAi
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;

    constexpr ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv, uint8_t av) : r{ rv }, g{ gv }, b{ bv }, a{ av } { }
    constexpr ColorRGBAi(uint8_t rv, uint8_t gv, uint8_t bv) : ColorRGBAi(rv, gv, bv, 255) { }
    constexpr ColorRGBAi() = default;

    ColorRGBAi(const ColorRGBi& v) : r{ v.r }, g{ v.g }, b{ v.b } { }
    ColorRGBi toRGB() const { return ColorRGBi(r, g, b); }
    explicit operator ColorRGBi() { return toRGB(); }

    // See ColorRGBAf for color constants
};
