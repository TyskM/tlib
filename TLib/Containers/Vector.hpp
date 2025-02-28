
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/vector.h>
#include <TLib/String.hpp>

template <typename T, typename Allocator = MiAllocator>
struct Vector : eastl::vector<T, Allocator>
{
    using eastl::vector<T, Allocator>::vector;

    template <typename T>
    bool validIndex(const T index) const
    {
        return index >= 0 && index < this->size();
    }

    std::string toString() const
    {
        if (this->empty())
        { return "[]"; }

        std::stringstream ss;
        ss << "[";
        ss << "(" << this->at(0).toString() << ")";
        for (size_t i = 1; i < this->size(); i++)
        { ss << ", (" << this->at(i).toString() << ")"; }
        ss << "]";

        return ss.str();
    }
};