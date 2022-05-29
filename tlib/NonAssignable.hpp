#pragma once

struct NonCopyable
{
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    void operator=(const NonCopyable&) = delete;
};

struct NonMoveable
{
    NonMoveable() = default;
    ~NonMoveable() = default;
    NonMoveable(NonMoveable&&) = delete;
    NonMoveable& operator=(NonMoveable&&) = delete;
};

struct NonAssignable : NonMoveable, NonCopyable { };