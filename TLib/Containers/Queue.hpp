
#pragma once
#include <TLib/EASTL.hpp>
#include <EASTL/queue.h>
#include <EASTL/priority_queue.h>
#include <TLib/Containers/Deque.hpp>
#include <TLib/Threading.hpp>

template <typename T>
using DefaultQueueContainer = Deque<T, AllocatorMiMalloc, DEQUE_DEFAULT_SUBARRAY_SIZE(T)>;

template <typename T, typename Container = DefaultQueueContainer<T>>
using Queue = eastl::queue<T, Container>;

template <typename T,
          typename Container = DefaultQueueContainer<T>,
          typename Compare   = eastl::less<typename Container::value_type>>
using PriorityQueue = eastl::priority_queue<T, Container, Compare>;

// Thread safe queue
template <typename T, typename Container = DefaultQueueContainer<T>>
class QueueMT : private Queue<T, Container>
{
    using This = Queue<T, Container>;
    Mutex mutex;

public:
    void lock()      { mutex.lock(); }
    void unlock()    { mutex.unlock(); }
    bool tryLock()   { return mutex.tryLock(); }
    auto lockGuard() { return LockGuard(mutex); }

    T pop()
    {
        lock();
        T value = This::front();
        This::pop();
        unlock();
        return value;
    }

    void push(const T& value)
    {
        lock();
        This::push(value);
        unlock();
    }

    bool empty()
    {
        lock();
        bool ret = This::empty();
        unlock();
        return ret;
    }

    size_t size()
    {
        lock();
        size_t ret = This::size();
        unlock();
        return ret;
    }
};