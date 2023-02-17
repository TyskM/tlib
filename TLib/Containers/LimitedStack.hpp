#pragma once

#include <deque>

template <typename T>
struct LimitedStack
{
protected:
    size_t maxSize;
    std::deque<T> cont;

public:
    LimitedStack(size_t maxSize) : maxSize{ maxSize } { }

    void push(const T& value)
    {
        cont.push_back(value);
        if (size() > maxSize) { pop(); }
    }

    void push(T&& value)
    {
        cont.push_back(std::move(value));
        if (size() > maxSize) { pop(); }
    }

    // Pops the top of the stack
    void pop() { cont.pop_back(); }

    T& top() { return cont.back(); }

    T& bottom() { return cont.front(); }

    void setMaxSize(size_t value)
    {
        maxSize = value;
        while (cont.size() > maxSize)
        { pop(); }
    }

    inline size_t size() const noexcept { return cont.size(); }

    inline void clear() noexcept { cont.clear(); }

    inline auto begin() noexcept { return cont.begin(); }
    inline auto end()   noexcept { return cont.end();   }
    inline auto begin() const noexcept { return cont.begin(); }
    inline auto end()   const noexcept { return cont.end(); }
};