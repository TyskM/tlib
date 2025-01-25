
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/vector.h>

template <typename T, typename Allocator = MiAllocator>
struct Vector : eastl::vector<T, Allocator>
{
    using eastl::vector<T, Allocator>::vector;

    template <typename T>
    bool validIndex(const T index) const
    {
        return index >= 0 && index < this->size();
    }
};