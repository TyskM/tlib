
#pragma once

#include <cstdint>

struct ColorRGBi
{
    constexpr ColorRGBi(uint8_t rv, uint8_t gv, uint8_t bv) : r{ rv }, g{ gv }, b{ bv } { }
    constexpr ColorRGBi() : r{ 0 }, g{ 0 }, b{ 0 } { }

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};
