#pragma once

#include <TLib/DataStructures.hpp>

struct Transform2D
{
    Vector2f pos;
    Vector2f scale = { 1, 1 };
    float    rot;

    Transform2D& operator+(const Transform2D& other)
    {
        Transform2D tf;
        tf.pos   = pos   + other.pos;
        tf.scale = scale * other.scale;
        tf.rot   = rot   + other.rot;
        return tf;
    }

    void operator +=(const Transform2D& other)
    { *this = *this + other; }
};