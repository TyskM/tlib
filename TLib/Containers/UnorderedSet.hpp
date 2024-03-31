
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/unordered_set.h>

template <typename Value,
          typename Hash       = eastl::hash<Value>,
          typename Predicate  = eastl::equal_to<Value>,
          typename Allocator  = MiAllocator,
          bool bCacheHashCode = false>
using UnorderedSet = eastl::unordered_set<Value, Hash, Predicate, Allocator, bCacheHashCode>;