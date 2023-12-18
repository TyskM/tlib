#pragma once

#include <TLib/DataStructures.hpp>
#include <TLib/Math.hpp>
#include <glm/ext.hpp>

struct View
{
    Vector2f center   = {0.f, 0.f};
    Vector2f size     = {1280, 720};
    Vector2f zoom     = {1.f, 1.f};
    float    rotation = 0.f; // Radians
    Rectf    viewport = {0.f, 0.f, 1.f, 1.f};

    void setBounds(const Rectf& rect)
    {
        center = rect.getPos() + rect.getSize() / 2.f;
        size = rect.getSize();
    }

    Vector2f topLeft() const
    {
        return center - (size / zoom / 2.f);
    }

    glm::mat4 getMatrix() const
    {
        Vector2f halfSize = size/2.f;
        const float l = center.x - halfSize.x;
        const float r = center.x + halfSize.x;
        const float t = center.y - halfSize.y;
        const float b = center.y + halfSize.y;

        glm::mat4 mat(1.f);
        mat = glm::scale(mat, {zoom.x, zoom.y, 1.f});

        glm::mat4 orthoMat = glm::ortho(l, r, b, t, -1.f, 1.f);

        return mat * orthoMat;
    }
};
