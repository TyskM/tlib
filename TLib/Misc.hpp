#pragma once

#include <iostream>
#include <TLib/Types/Types.hpp>

static inline Rectf getBestImageRect(Vector2f targetSize, Vector2f imgSize)
{
    auto screenWidth = targetSize.x;
    auto screenHeight = targetSize.y;

    auto imgWidth = imgSize.x;
    auto imgHeight = imgSize.y;

    //auto imgRatio = imgWidth / imgHeight;
    //auto screenRatio = screenWidth / screenHeight;

    auto bestRatio = std::min(screenWidth / imgWidth, screenHeight / imgHeight);

    auto newImgWidth = imgWidth * bestRatio;
    auto newImgHeight = imgHeight * bestRatio;

    auto pos = Vector2f(screenWidth - newImgWidth, screenHeight - newImgHeight) / 2.f;

    return Rectf(pos.x, pos.y, newImgWidth, newImgHeight);
}

template <typename T>
static bool containsAny(const std::vector<T>& vec, const T& value)
{
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

// https://cplusplus.com/forum/general/188061/
template<class... Types>
static void printCSVs(Types... args)
{
    bool first = true;

    for (auto&& x : { args... })
    {
        if (!first) { std::cout << ", "; }
        std::cout << x;
        first = false;
    }

    std::cout << std::endl;
}

static Vector2i posToGridPos(Vector2f pos, Vector2f gridSize)
{
    // Integer division truncates towards 0
    // Floor manually so it doesn't break with negative values
    return Vector2i((pos / gridSize).floored());
}
