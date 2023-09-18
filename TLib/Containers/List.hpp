
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/list.h>

template <typename T, typename Allocator = MiAllocator>
using List = eastl::list<T, Allocator>;