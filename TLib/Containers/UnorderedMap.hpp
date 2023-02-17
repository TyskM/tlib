
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/unordered_map.h>

template <typename Key,
          typename T,
          typename Hash           = eastl::hash<Key>,
          typename Predicate      = eastl::equal_to<Key>,
          typename Allocator      = MiAllocator,
          bool     bCacheHashCode = false>
using UnorderedMap = eastl::unordered_map<Key, T, Hash, Predicate, Allocator, bCacheHashCode>;