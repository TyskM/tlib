
#pragma once
#include <TLib/Types/Types.hpp>
#include <TLib/thirdparty/multiarray/array.h>

template <typename T>
class GridMap2D
{
public:
    using Array2D = nda::shape<nda::dim<>, nda::dim<>>;
    template <typename T>
    using Grid2D = nda::array<T, Array2D>;
    using SizeType = int32_t;

private:
    Grid2D<T> grids;

public:

    GridMap2D() = default;
    GridMap2D(const Vector2i& size)   { resize(size); }
    GridMap2D(SizeType x, SizeType y) { resize(x, y); }

    SizeType width()  const { return static_cast<SizeType>(grids.width()); }
    SizeType height() const { return static_cast<SizeType>(grids.height()); }
    Vector2i size()   const { return Vector2i(static_cast<SizeType>(grids.width()), static_cast<SizeType>(grids.height())); }

    bool inBounds(SizeType x, SizeType y) const
    {
        return 0 <= x && x < width()
            && 0 <= y && y < height();
    }

    bool inBounds(const Vector2i& pos) const
    {
        return inBounds(pos.x, pos.y);
    }

    void resize(SizeType x, SizeType y) { grids.reshape(Array2D(x, y)); }
    void resize(const Vector2i& size)   { resize(size.x, size.y); }

    void clear()
    {
        for (SizeType x = 0; x < grids.width();  x++) {
        for (SizeType y = 0; y < grids.height(); y++)
        {
            grids(x, y) = T();
        }}
    }

    void clear(const T& grid)
    {
        for (SizeType x = 0; x < grids.width();  x++) {
        for (SizeType y = 0; y < grids.height(); y++)
        {
            grids(x, y) = grid;
        }};
    }

          T& at(const Vector2i& pos)          { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
          T& at(SizeType x, SizeType y)       { return at({ x, y }); }
    const T& at(const Vector2i& pos)    const { ASSERT(inBounds(pos)); return grids(pos.x, pos.y); }
    const T& at(SizeType x, SizeType y) const { return at({ x, y }); }

    Vector<Vector2i> circle(const Vector2i& center, float radius)
    {
        Vector<Vector2i> ret;

        // https://www.redblobgames.com/grids/circle-drawing/
        SizeType top    =  ceil(center.y - radius),
                 bottom = floor(center.y + radius),
                 left   =  ceil(center.x - radius),
                 right  = floor(center.x + radius);

        for (SizeType y = top;  y <= bottom; y++) {
        for (SizeType x = left; x <= right;  x++)
        {
            bool inbounds = inBounds(Vector2i(x, y));
            if (!inbounds) { continue; }

            bool inRange = (Vector2f(center).distanceToSquared(Vector2f(x, y)) <= radius);
            if (!inRange) { continue; }

            ret.emplace_back(x, y);
        }}
        return ret;
    }
};
