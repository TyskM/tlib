#pragma once

#include <vector>
#include "../Pointers.hpp"

struct GameObj : SafeObj
{
    bool freed = false;
};

// Game Object Container
template <typename T, typename ContainerType = std::vector<T>>
struct GOContainer
{
    static_assert(std::is_base_of<GameObj, T>::value,
                  "T must derive from GameObj");

    ContainerType data;

    void clearFreed()
    {
        std::erase_if(data, [](const T& v) { return v.freed; });
    }
};