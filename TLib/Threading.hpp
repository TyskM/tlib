#pragma once

#include "NonAssignable.hpp"
#include <mutex>
#include <thread>

struct Mutex : protected std::mutex
{
    // https://stackoverflow.com/questions/21892934/how-to-assert-if-a-stdmutex-is-locked

    void lock()
    {
        std::mutex::lock();
        holder = std::this_thread::get_id();
    }

    void unlock()
    {
        holder = std::thread::id();
        std::mutex::unlock();
    }

    bool tryLock()
    {
        if (std::mutex::try_lock())
        { holder = std::this_thread::get_id(); return true; }
        return false;
    }

    // @return true if the mutex is locked by the caller of this method
    bool lockedByCaller() const noexcept
    {
        return holder == std::this_thread::get_id();
    }

    std::atomic<std::thread::id> holder;

};

struct LockGuard
{
    Mutex& mutex;

    LockGuard(Mutex& mutex) : mutex{ mutex }
    { mutex.lock(); }

    ~LockGuard() noexcept
    { mutex.unlock(); }
};

template <typename T>
struct OptionalLockGuard : NonAssignable
{
    T* mutex = nullptr;

    OptionalLockGuard(T& mutex, bool use = true)
    {
        if (use)
        {
            this->mutex = &mutex;
            _Acquires_lock_(mutex.lock());
        }
    }

    ~OptionalLockGuard()
    {
        if (mutex) { _Releases_lock_(mutex->unlock()); }
    }
};

template <typename T>
using Atomic = std::atomic<T>;

using Thread = std::thread;