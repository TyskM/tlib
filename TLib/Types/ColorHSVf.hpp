
#pragma once

struct ColorHSVf
{
    float h = 0.f;
    float s = 0.f;
    float v = 0.f;

    constexpr ColorHSVf() = default;
    constexpr ColorHSVf(float h, float s, float v) : h{ h }, s{ s }, v{ v } { }
};
