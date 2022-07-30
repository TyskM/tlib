#pragma once

#include <box2d/b2_draw.h>
#include <vector>
#include "Renderer.hpp"

constexpr int B2DEBUG_ALLFLAGS = b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit;

struct B2DebugDraw : public b2Draw
{
    Renderer* renderer = nullptr;

    B2DebugDraw(Renderer& renderer) : renderer{&renderer} { create(renderer); }
    B2DebugDraw() = default;

    void create(Renderer& renderer, int flags = B2DEBUG_ALLFLAGS)
    {
        this->renderer = &renderer;
        SetFlags(flags);
    }

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
    {
        renderer->drawPolygon(vertices, vertexCount, ColorRGBAi(ColorRGBAf{color.r, color.g, color.b, color.a}));
    }

    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
    {
        renderer->drawFilledPolygon(vertices, vertexCount, ColorRGBAi(ColorRGBAf{color.r, color.g, color.b, color.a}));
    }

    void DrawCircle(const b2Vec2& center, float_t radius, const b2Color& color)
    {

    }

    void DrawSolidCircle(const b2Vec2& center, float_t radius, const b2Vec2& axis, const b2Color& color)
    {

    }

    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
    {
        renderer->drawLine(p1, p2, {color.r, color.g, color.b, color.a});
    }

    void DrawTransform(const b2Transform& xf)
    {

    }

    void DrawPoint(const b2Vec2& p, float size, const b2Color& color)
    {

    }

};