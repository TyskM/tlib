
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/queue.h>
#include <EASTL/priority_queue.h>
#include <TLib/Containers/Deque.hpp>

template <typename T>
using DefaultQueueContainer = Deque<T, MiAllocator, DEQUE_DEFAULT_SUBARRAY_SIZE(T)>;

template <typename T, typename Container = DefaultQueueContainer<T>>
using Queue = eastl::queue<T, Container>;

template <typename T,
          typename Container = DefaultQueueContainer<T>,
          typename Compare   = eastl::less<typename Container::value_type>>
using PriorityQueue = eastl::priority_queue<T, Container, Compare>;