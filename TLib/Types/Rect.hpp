
#pragma once

#include <TLib/Types/Vector2.hpp>
#include <TLib/Types/Circle.hpp>

// This rect is meant to represent position + size
template<typename T = float>
struct Rect
{
    T x      = 0;
    T y      = 0;
    T width  = 0;
    T height = 0;

    Rect(T x, T y, T width, T height) : x{ x }, y{ y }, width{ width }, height{ height } { }

    Rect(const Vector2<T>& pos, const Vector2<T>& size) : x{ pos.x }, y{ pos.y }, width{ size.x }, height{ size.y } { }

    Rect(const Vector2<T>& pos, T width, T height) : x{ pos.x }, y{ pos.y }, width{ width }, height{ height } { }

    Rect(T x, T y, const Vector2<T>& size) : x{ x }, y{ y }, width{ size.x }, height{ size.y } { }

    Rect() = default;

    template <typename CT>
    constexpr explicit Rect(const CT& other)
    {
        x      = static_cast<T>(other.x);
        y      = static_cast<T>(other.y);
        width  = static_cast<T>(other.width);
        height = static_cast<T>(other.height);
    }

    static Rect fromLTRB(T left, T top, T right, T bottom)
    {
        if (right < left) { std::swap(right, left); }
        if (bottom < top) { std::swap(bottom, top); }
        return Rect<T>(left, top, right - left, bottom - top);
    }

    static Rect fromLTRB(const Vector2<T>& lt, const Vector2<T>& rb)
    { return fromLTRB(lt.x, lt.y, rb.x, rb.y); }

    Vector2<T> getPos() const
    { return Vector2<T>(x, y); }

    Vector2<T> getSize() const
    { return Vector2<T>(width, height); }

    void setPos(const Vector2<T>& v)
    { x = v.x; y = v.y; }

    void setSize(const Vector2<T>& v)
    { width = v.x; height = v.y; }

    T getRight() const
    { return x + width; }

    T getBottom() const
    { return y + height; }

    Vector2<T> getRightBottom() const
    { return { getRight(), getBottom() }; }

    Vector2<T> getTopRight() const
    { return { getRight(), y }; }

    Vector2<T> getBottomLeft() const
    { return { x, getBottom() }; }

    Vector2<T> center() const
    { return getPos() + (getSize() / T(2)); }

    bool contains(Vector2<T> value) const
    { return contains(value.x, value.y); }

    bool contains(const T& x, const T& y) const
    {
        Vector2<T> topLeft ={ this->x, this->y };
        Vector2<T> bottomRight = topLeft + Vector2<T>{ width, height };

        return topLeft.x < x && x < bottomRight.x && topLeft.y < y && y < bottomRight.y;
    }

    bool intersects(const Rect<T>& otherRect) const
    {
        Vector2<T> bottomRight = Vector2<T>{ x, y } + Vector2<T>{ width, height };
        Vector2<T> otherBottomRight = Vector2<T>{ otherRect.x, otherRect.y } +
            Vector2<T>{ otherRect.width, otherRect.height };

        T x1, x2, y1, y2;
        T ox1, ox2, oy1, oy2;

        x1 = x; y1 = y;
        x2 = x + width;
        y2 = y + height;

        ox1 = otherRect.x;
        oy1 = otherRect.y;
        ox2 = otherRect.x + otherRect.width;
        oy2 = otherRect.y + otherRect.height;

        return !(x1  >= ox2 ||
            ox1 >= x2  ||
            y1  >= oy2 ||
            oy1 >= y2);
    }

    bool intersects(const Circle<T>& circle)
    {
        // https://stackoverflow.com/questions/401847/circle-rectangle-collision-detection-intersection
        // Find the closest point to the circle within the rectangle
        float closestX = math::clamp<T>(circle.x, x, x + width);
        float closestY = math::clamp<T>(circle.y, y, y + height);

        // Calculate the distance between the circle's center and this closest point
        float distanceX = circle.x - closestX;
        float distanceY = circle.y - closestY;

        // If the distance is less than the circle's radius, an intersection occurs
        float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
        return distanceSquared <= (circle.radius * circle.radius);
    }

    String toString() const { return fmt::format("({}, {}, {}, {})"); }
    operator String() const { return toString(); }

    bool operator==(const Rect<T>& other)
    { return (x == other.x && y == other.y && width == other.width && height == other.height); }

    bool operator!=(const Rect<T>& other)
    { return !(operator==(other)); }
};

using Rectf = Rect<float>;
using Recti = Rect<int>;
