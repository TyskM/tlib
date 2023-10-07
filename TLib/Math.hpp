
#pragma once
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace math
{
    template<class T>
    inline T rad2deg(const T& rad)
    { return rad * (180 / glm::pi<T>()); }

    template<class T>
    inline T deg2rad(const T& deg)
    { return deg * (glm::pi<T>() / 180); }

    template<class T>
    const auto clamp = std::clamp<T>;

    template<class T>
    inline T stepify(const T& value, const T& step)
    {
        if (value == 0.f) { return value; }
        return ( floor(value / step) * step );
    }

    template<class T>
    inline T stepifyRound(const T& value, const T& step)
    {
        if (value == 0.f) { return value; }
        return (round(value / step) * step);
    }

    template<class T>
    float normalize(const T& val, const T& min, const T& max)
    { return (val - min) / (max - min); }

    template <class T>
    int sign(const T& value)
    { return (value > 0) - (value < 0); }

    template <typename T>
    float lerpRotDegrees(T start, T end, T amount)
    {
        // https://stackoverflow.com/questions/2708476/rotation-interpolation
        T difference = abs(end - start);
        if (difference > 180)
        {
            // We need to add on to one of the values.
            if (end > start)
            { start += 360; } else
            { end += 360; }
        }

        T value = (start + ((end - start) * amount));
        T rangeZero = 360;

        if (value >= 0 && value <= 360)
        { return value; }

        return fmod(value, rangeZero);
    }

    template <typename T>
    T lerpRotRads(T start, T end, T amount)
    {
        // https://stackoverflow.com/questions/2708476/rotation-interpolation
        T difference = abs(end - start);
        const T r180 = glm::pi<T>();
        const T r360 = r180 * 2;
        if (difference > r180)
        {
            // We need to add on to one of the values.
            if (end > start)
            { start += r360; } else
            { end += r360; }
        }

        T value = (start + ((end - start) * amount));
        T rangeZero = r360;

        if (value >= 0 && value <= r360)
        { return value; }

        return fmod(value, rangeZero);
    }
}