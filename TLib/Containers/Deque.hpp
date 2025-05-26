

#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/deque.h>

template <typename T, typename Allocator = AllocatorMiMalloc, unsigned SubarraySize = DEQUE_DEFAULT_SUBARRAY_SIZE(T)>
using Deque = eastl::deque<T, Allocator, SubarraySize>;