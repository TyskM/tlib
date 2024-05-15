
#pragma once

#include <TLib/Types/Vector2.hpp>

template<typename T = float>
struct Circle
{
    T x, y, radius;

    constexpr Circle(T x, T y, T radius) : x{ x }, y{ y }, radius{ radius } { }

    bool intersects(const Circle<T>& other)
    { return Vector2<T>(x, y).distanceTo(Vector2<T>(other.x, other.y)) < other.radius + radius; }

    bool contains(T px, T py)
    { return std::pow(px-x, 2) + std::pow(py-y, 2) < std::pow(radius, 2); }

    bool contains(const Vector2<T>& v)
    { return contains(v.x, v.y); }
};

using Circlef = Circle<float>;
using Circlei = Circle<int>;
