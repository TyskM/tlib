#pragma once

#include "../DataStructures.hpp"
#include "../Math.hpp"
#include <glm/ext.hpp>


struct View
{
    Rectf bounds;
    float zoom = 1.f; // not yet implemented
    float rot = 0.f; // degrees // not yet implemented

    View(float x, float y, float width, float height) : bounds{x, y, width, height} { }
    View() = default;

    Vector2f localToWorldPos(const Vector2i& winSize, const Vector2f& pos) const
    {
        Vector2f scale = Vector2(bounds.width, bounds.height) / Vector2f(winSize);
        return pos * scale + Vector2f{bounds.x, bounds.y};
    }

    Vector2f worldToLocalPos(const Vector2f& pos) const
    {
        return pos - Vector2f{bounds.x, bounds.y};
    }

    glm::mat4 getMatrix() const
    {
        const float l = bounds.x;
        const float r = bounds.x + bounds.width;
        const float t = bounds.y;
        const float b = bounds.y + bounds.height;
        const auto mat = glm::ortho(l, r, b, t, -1.f, 1.f);
        return mat;
    }
};