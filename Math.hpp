#pragma once

#include <algorithm>

namespace math
{
    const double pi = 3.141592653589793238463;

    template<class T>
    inline T rad2deg(const T& rad)
    { return rad * (180 / pi); }

    template<class T>
    inline T deg2rad(const T& deg)
    { return deg * (pi / 180); }

    template<class T>
    const auto clamp = std::clamp<T>;

    template<class T>
    inline T stepify(const T& value, const T& step)
    {
        // return ( floor(value / step) * step );
        return step * std::round(value / step);
    }

    template<class T>
    float normalize(const T& val, const T& min, const T& max)
    {
        return (val - min) / (max - min);
    }

    template <class T>
    int sign(const T& value)
    { return (value > 0) - (value < 0); }
}