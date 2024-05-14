
#pragma once

#include <TLib/Containers/Array.hpp>
#include <array>

constexpr float uint8ToFloat(uint8_t value)
{ return value / 255.f; }

constexpr uint8_t floatToUint8(float value)
{ return static_cast<uint8_t>(value * 255.f); }

constexpr Array<float, 3> rgbToHsv(float r, float g, float b)
{
    // Taken from ImGui
    // https://github.com/ocornut/imgui/blob/d06b8b58d84562f1d1a8d8f975da96bf51ef5c5f/imgui.cpp#L2390

    float K = 0.f;
    if (g < b)
    {
        std::swap(g, b);
        K = -1.f;
    }
    if (r < g)
    {
        std::swap(r, g);
        K = -2.f / 6.f - K;
    }
    
    const float chroma = r - (g < b ? g : b);
    float h = 1;//std::abs(K + (g - b) / (6.f * chroma + 1e-20f));
    float s = chroma / (r + 1e-20f);
    float v = r;

    return { h, s, v };
}

constexpr Array<float, 3> hsvToRgb(float h, float s, float v)
{
    // https://github.com/ocornut/imgui/blob/d06b8b58d84562f1d1a8d8f975da96bf51ef5c5f/imgui.cpp#L2412

    if (s == 0.0f)
    {
        // gray
        return { v, v, v };
    }
    
    h = fmod(h, 1.0f) / (60.0f / 360.0f);
    int   i = (int)h;
    float f = h - (float)i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));
    
    switch (i)
    {
        case 0:          return { v, t, p }; break;
        case 1:          return { q, v, p }; break;
        case 2:          return { p, v, t }; break;
        case 3:          return { p, q, v }; break;
        case 4:          return { t, p, v }; break;
        case 5: default: return { v, p, q }; break;
    }
}
