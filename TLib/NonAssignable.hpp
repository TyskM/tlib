#pragma once

// Disable copy
struct NonCopyable
{
    NonCopyable()                               = default;
    ~NonCopyable()                              = default;
    NonCopyable(const NonCopyable&)    noexcept = delete;
    void operator=(const NonCopyable&) noexcept = delete;
};

// Disable move
struct NonMoveable
{
    NonMoveable()                                  = default;
    ~NonMoveable()                                 = default;
    NonMoveable(NonMoveable&&)            noexcept = delete;
    NonMoveable& operator=(NonMoveable&&) noexcept = delete;
};

// Disable move and copy
struct NonAssignable : NonMoveable, NonCopyable { };