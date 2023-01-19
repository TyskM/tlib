#pragma once

#include "../Math.hpp"
#include "../Macros.hpp"

template <typename T = int>
struct Resource
{
    T min;
    T max;
    T value;

    Resource(T min, T max, T value) : min{min}, max{max}, value{value}
    {
        ASSERT(max >= min);
        this->value = std::clamp(value, min, max);
    }

    Resource(T min, T max) : Resource(min, max, max) { }
    Resource(T max)        : Resource(0, max, max)   { }

    inline void setMaxAndValue(T v) noexcept { setMax(v); set(v); }
    inline void setMin(T v) noexcept { min = v; set(value); }
    inline void setMax(T v) noexcept { max = v; set(value); }
    inline void set   (T v) noexcept { value = std::clamp(v, min, max); }
    inline void add   (T v) noexcept { set(value + v); }
    inline void reduce(T v) noexcept { set(value - v); }
    inline bool full()      noexcept { return value == max; }
    inline bool depleted()  noexcept { return value == min; }
};