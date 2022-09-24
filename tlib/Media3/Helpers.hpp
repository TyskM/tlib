#pragma once

#include "../DataStructures.hpp"

Vector2i posToGridPos(Vector2f pos, Vector2f gridSize)
{
    // Integer division truncates towards 0
    // Floor manually so it doesn't break with negative values
    return Vector2i((pos / gridSize).floored());
}