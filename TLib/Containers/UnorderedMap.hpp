
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/unordered_map.h>

template <typename Key,
          typename T,
          typename Hash           = eastl::hash<Key>,
          typename Predicate      = eastl::equal_to<Key>,
          typename Allocator      = MiAllocator,
          bool     bCacheHashCode = false>
struct UnorderedMap : public eastl::unordered_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>
{
    using eastl::unordered_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>::unordered_map;

    bool contains(const Key& key) const
    {
        auto it = this->find(key);
        return (it != this->end());
    }
};
