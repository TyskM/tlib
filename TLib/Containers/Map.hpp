
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/map.h>

template <typename Key,
          typename T,
          typename Compare = eastl::less<Key>,
          typename Allocator = MiAllocator>
struct Map : public eastl::map<Key, T, Compare, Allocator>
{
    using eastl::map<Key, T, Compare, Allocator>::map;

    bool contains(const Key& key) const
    {
        auto it = this->find(key);
        return (it != this->end());
    }
};
