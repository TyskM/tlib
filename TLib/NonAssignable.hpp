#pragma once

// Disable copy
struct NonCopyable
{
    NonCopyable()                                       = default;
    ~NonCopyable()                                      = default;
    NonCopyable(const NonCopyable&)            noexcept = delete;
    NonCopyable& operator=(const NonCopyable&) noexcept = delete;
};

// Disable move
struct NonMoveable
{
    NonMoveable()                                  = default;
    ~NonMoveable()                                 = default;
    NonMoveable(NonMoveable&&)            noexcept = delete;
    NonMoveable& operator=(NonMoveable&&) noexcept = delete;
};

struct Moveable
{
    Moveable()                               = default;
    ~Moveable()                              = default;
    Moveable(Moveable&&)            noexcept = default;
    Moveable& operator=(Moveable&&) noexcept = default;
};

// Disable move and copy
struct NonAssignable : NonMoveable, NonCopyable { };