

#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/deque.h>

template <typename T, typename Allocator = MiAllocator>
using Deque = eastl::deque<T, Allocator>;