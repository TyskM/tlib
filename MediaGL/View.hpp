#pragma once

#include "../DataStructures.hpp"
#include "../Math.hpp"
#include <glm/ext.hpp>

struct View
{
    Rectf bounds = {0,0,500,500};
    float rot = 0.f; // degrees // not yet implemented
    Vector2f zoom = {1.f, 1.f};

    View(float x, float y, float width, float height) : bounds{x, y, width, height} { }
    View() = default;

    glm::mat4 getMatrix() const
    {
        Rectf tempBounds = bounds;

        const float l = tempBounds.x;
        const float r = tempBounds.x + tempBounds.width;
        const float t = tempBounds.y;
        const float b = tempBounds.y + tempBounds.height;

        glm::mat4 mat(1.f);
        mat = glm::scale(mat, { zoom.x, zoom.y, 1.f });
        

        glm::mat4 orthoMat = glm::ortho(l, r, b, t, -1.f, 1.f);
        
        return mat * orthoMat;
    }
};