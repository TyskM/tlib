
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/queue.h>
#include <TLib/Containers/Deque.hpp>

template <typename T, typename Container = Deque<T, MiAllocator, DEQUE_DEFAULT_SUBARRAY_SIZE(T)> >
using Queue = eastl::queue<T, Container>;