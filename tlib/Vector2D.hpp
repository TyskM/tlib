#pragma once

#ifdef _DEBUG
    #include <iostream>
#endif

#include <vector>

#include <iterator>

// https://stackoverflow.com/questions/12993403/accessing-a-vectorvectorint-as-a-flat-array

// A flat 2D std::vector
template <typename Type>
struct Vector2D
{
private:
    std::vector<Type> items;
    size_t x = 0;
    size_t y = 0;

public:
    struct Size2D
    {
        size_t x, y;
        Size2D(size_t x, size_t y) { this->x = x; this->y = y; }
    };

    Vector2D() { }
    Vector2D(size_t x, size_t y) { resize(x, y); }

    void reserve(size_t x, size_t y)
    {
        items.reserve(x * y);
    }

    void resize(size_t x, size_t y)
    {
        this->x = x; this->y = y;
        items.resize(x * y);
    }

    void fill(Type point2)
    {
        for (size_t i = 0; i < items.size(); i++)
        { items[i] = point2; }
    }

    Type get(size_t x, size_t y) const
    { return items[indexFor(x, y)]; }

    void set(size_t x, size_t y, Type value)
    { items[indexFor(x, y)] = value; }

    size_t indexFor(size_t x, size_t y) const noexcept
    { return y * x + x; }

    void clear() { items.clear(); }

    inline auto data() noexcept { return items.data(); }

    inline bool empty() const noexcept { return items.empty(); }
    inline size_t capacity() const noexcept { return items.capacity(); }
    inline size_t max_size() const noexcept { return items.max_size(); }
    inline size_t size() const noexcept { return items.size(); }
    inline Size2D size2d() const noexcept { return Size2D(x, y); }

    inline auto begin() noexcept { return items.begin(); }
    inline auto end()   noexcept { return items.end(); }
    inline auto rbegin() noexcept { return items.rbegin(); }
    inline auto rend()   noexcept { return items.rend(); }
    inline auto cbegin() const noexcept { return items.cbegin(); }
    inline auto cend()   const noexcept { return items.cend(); }
    inline auto crbegin() const noexcept { return items.crbegin(); }
    inline auto crend()   const noexcept { return items.crend(); }
    inline Type front() const noexcept { return items.front(); }
    inline Type back()  const noexcept { return items.back(); }


    void printContents()
    {
        for (size_t yv = 0; yv < y; yv++)
        {
            std::cout << "\n";
            for (size_t xv = 0; xv < x; xv++)
            {
                std::cout << get(xv, yv) << ", ";
            }
        }
    }

};