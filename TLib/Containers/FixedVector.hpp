
#pragma once
#include <EASTL/fixed_vector.h>

template <typename T,
    size_t   count,
    bool     allowOverflow     = false,
    typename OverflowAllocator = typename eastl::conditional<allowOverflow, EASTLAllocatorType, EASTLDummyAllocatorType>::type>
using FixedVector = eastl::fixed_vector<T, count, allowOverflow, OverflowAllocator>;
