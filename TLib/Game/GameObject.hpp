#pragma once

#include <TLib/Containers/Vector.hpp>
#include <TLib/Pointers.hpp>

struct GameObj : SafeObj
{
    bool freed = false;
};

// Game Object Container
template <typename T, typename ContainerType = Vector<T>>
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