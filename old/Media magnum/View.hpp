#pragma once

#include "../../DataStructures.hpp"

struct View
{
    Vector2f center;
    Vector2f size;
    float rot = 0.f; // in degrees

    inline Vector2f getTopLeftPos() const noexcept
    {
        return { center.x - size.x / 2, center.y + size.y / 2 };
    }
};