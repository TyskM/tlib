#pragma once

#include <iostream>
#include "DataStructures.hpp"

template <typename T>
struct Flat2DVector
{
    std::vector<T> vector;
    size_t width, height;

    Flat2DVector() { }
    Flat2DVector(size_t width, size_t height) : width{ width }, height{ height }
    {
        vector.reserve(width * height);
    }

    size_t getIndex(size_t x, size_t y) const
    {
        ASSERT(width > 0) // You forgot to init the vector, use the constructor, call clear(x, y), or call resize.
            auto index = y * width + x;
        ASSERT(index < vector.size()); // Out of bounds
        return index;
    }

    void set(size_t x, size_t y, const T& value)
    { vector[getIndex(x, y)] = value; }

    void set(const Vector2i& xy, const T& value)
    { set(xy.x, xy.y, value); }

    template <typename... Args>
    void emplace(size_t x, size_t y, Args... args)
    { vector.emplace(vector.begin() + getIndex(x, y), args...); }

    template <typename... Args>
    void emplace(const Vector2i& xy, Args... args)
    { emplace(xy.x, xy.y, args); }

    T& get(size_t x, size_t y)
    { return vector[getIndex(x, y)]; }

    T& get(const Vector2i& xy)
    { return get(xy.x, xy.y); }

    const T& get(size_t x, size_t y) const
    { return vector[getIndex(x, y)]; }

    const T& get(const Vector2i& xy) const
    { return get(xy.x, xy.y); }

    void clear(size_t x, size_t y)
    {
        vector.clear();
        vector.resize(x * y);
    }

    void clear()
    { vector.clear(); }

    void resize(size_t width, size_t height)
    {
        std::vector<T> newv;
        newv.reserve(width * height);

        for (size_t nx = 0; nx < width; nx++)
        {
            for (size_t ny = 0; ny < height; ny++)
            {
                newv[ny * width + nx] = get(nx, ny);
            }
        }

        this->width  = width;
        this->height = height;
        this->vector.swap(newv);
    }

    auto begin()       noexcept { return vector.begin(); }
    auto end()         noexcept { return vector.end(); }
    auto data()        noexcept { return vector.data(); }
    auto front()       noexcept { return vector.front(); }
    auto back()        noexcept { return vector.back(); }
    auto begin() const noexcept { return vector.begin(); }
    auto end()   const noexcept { return vector.end(); }
    auto data()  const noexcept { return vector.data(); }
    auto front() const noexcept { return vector.front(); }
    auto back()  const noexcept { return vector.back(); }
};

static inline Rectf getBestImageRect(Vector2f targetSize, Vector2f imgSize)
{
    auto screenWidth = targetSize.x;
    auto screenHeight = targetSize.y;

    auto imgWidth = imgSize.x;
    auto imgHeight = imgSize.y;

    auto imgRatio = imgWidth / imgHeight;
    auto screenRatio = screenWidth / screenHeight;

    auto bestRatio = std::min(screenWidth / imgWidth, screenHeight / imgHeight);

    auto newImgWidth = imgWidth * bestRatio;
    auto newImgHeight = imgHeight * bestRatio;

    auto pos = Vector2f(screenWidth - newImgWidth, screenHeight - newImgHeight) / 2.f;

    return Rectf(pos.x, pos.y, newImgWidth, newImgHeight);
}

template <typename T>
static bool containsAny(const std::vector<T>& vec, const T& value)
{
    return std::find(vec.begin(), vec.end(), value) != vec.end();
}

// https://cplusplus.com/forum/general/188061/
template<class... Types>
static void printCSVs(Types... args)
{
    bool first = true;

    for (auto&& x : { args... })
    {
        if (!first) { std::cout << ", "; }
        std::cout << x;
        first = false;
    }

    std::cout << std::endl;
}
