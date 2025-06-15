
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/vector.h>
#include <TLib/Macros.hpp>
#include <TLib/Containers/Array.hpp>

template <typename T, typename Allocator = AllocatorMiMalloc>
struct Vector : eastl::vector<T, Allocator>
{
    static constexpr size_t NotFound = std::numeric_limits<size_t>::max();
    using eastl::vector<T, Allocator>::vector;
    using Base = eastl::vector<T, Allocator>;
    using This = Vector<T, Allocator>;

    template <typename T>
    bool validIndex(const T index) const
    {
        return index >= 0 && index < this->size();
    }

    bool eraseByRef(T& target)
    {
        for (size_t i = 0; i < this->size(); i++)
        {
            if ((*this)[i] == target)
            { this->erase(this->begin() + i); return true; }
        }
        return false;
    }

    using eastl::vector<T, Allocator>::push_back;

    void push_back(const This& other)
    { this->insert(this->end(), other.begin(), other.end()); }

    template <size_t Size>
    void push_back(const Array<T, Size>& other)
    { this->insert(this->end(), other.begin(), other.end()); }

    size_t tryFindIndex(T& target)
    {
        for (size_t i = 0; i < this->size(); i++)
        {
            if (target == (*this)[i])
            { return i; }
        }
        return NotFound;
    }

    T& find(T& target)
    {
        for (auto& value : *this)
        {
            if (target == value)
            { return value; }
        }
        ASSERT(false);
    }

    T* tryFind(T& target)
    {
        for (auto& value : *this)
        {
            if (target == value)
            { return &value; }
        }
        return nullptr;
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