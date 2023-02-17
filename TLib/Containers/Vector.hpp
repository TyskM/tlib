
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/vector.h>

template <typename T, typename Allocator = MiAllocator>
using Vector = eastl::vector<T, Allocator>;