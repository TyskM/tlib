
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/set.h>

template <typename Key, typename Compare = eastl::less<Key>, typename Allocator = AllocatorMiMalloc>
using Set = eastl::set<Key, Compare, Allocator>;