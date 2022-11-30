#pragma once

#include "../DataStructures.hpp"
#include <glm/ext.hpp>

struct View
{
    Rectf bounds;
    float zoom = 1.f;
    float rot = 0.f; // degrees

    View(float x, float y, float width, float height) : bounds{x, y, width, height} { }
    View() = default;

    Vector2f localToWorldPos(Vector2f pos) const
    {
        return pos + Vector2f{bounds.x, bounds.y};
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