
#pragma once

struct ColorRGBf;

struct ColorHSVf
{
    float h = 0.f;
    float s = 0.f;
    float v = 0.f;

    constexpr ColorHSVf() = default;
    ColorHSVf(const ColorRGBf& rgb);
};
