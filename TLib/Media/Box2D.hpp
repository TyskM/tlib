
#pragma once
#include <TLib/Logging.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Containers/Vector.hpp>
#include <box2d/box2d.h>

//// Misc

b2Vec2 tob2vec(const Vector2f& v)
{ return {v.x, v.y}; }

///// Raycast

namespace tlibdetail
{
    class RaycastFirstHandler : public b2RayCastCallback
    {
    public:
        bool       hit     = false;
        Vector2f   point;
        Vector2f   normal;
        b2Fixture* fixture = nullptr;

        float ReportFixture(b2Fixture* _fixture, const b2Vec2& _point, const b2Vec2& _normal, float _fraction) override
        {
            hit     = true;
            point   = Vector2f(_point);
            normal  = Vector2f(_normal);
            fixture = _fixture;
            return _fraction;
        }

        void reset()
        {
            hit = false;
        }
    } raycastFirstHandler;

    class AABBQueryAll : public b2QueryCallback
    {
    public:
        Vector<b2Body*> foundBodies;

        bool ReportFixture(b2Fixture* fixture) override
        {
            foundBodies.push_back(fixture->GetBody());
            return true;
        }

        void reset()
        { foundBodies.clear(); }

    } aabbQueryAll;
}

struct Intersection
{
    b2Fixture* fixture = nullptr;
    Vector2f   point;
    Vector2f   normal;
};

struct RaycastFirst
{
    bool hit = false;
    Intersection intersec; // If hit is false, ignore
};

RaycastFirst raycastFirst(const b2World& world, const Vector2f& p1, const Vector2f& p2)
{
    auto& rh = tlibdetail::raycastFirstHandler;
    rh.reset();
    world.RayCast(&rh, tob2vec(p1), tob2vec(p2));

    RaycastFirst ret;
    ret.hit = rh.hit;
    if (rh.hit)
    {
        ret.intersec.point   = rh.point;
        ret.intersec.normal  = rh.normal;
        ret.intersec.fixture = rh.fixture;
    }
    else
    { ret.intersec.point = p2; }
    return ret;
}

// Returned vector can be invalidated after calling any function in this header.
// Make a copy if you need it.
Vector<b2Body*>& queryAABB(const b2World& world, const b2AABB& aabb)
{
    auto& q = tlibdetail::aabbQueryAll;
    q.reset();
    world.QueryAABB(&q, aabb);
    return q.foundBodies;
}

////// Debug Draw

class TLibBox2DDebugDraw : public b2Draw
{
private:
    Vector2f convVec(const b2Vec2& vec)
    {
        return Vector2f(vec.x * scale, vec.y * scale);
    }

    ColorRGBAf convColor(const b2Color& color) const
    {
        return {color.r, color.g, color.b, color.a};
    }

    Vector<Vector2f> verts;

public:
    static inline uint32 allBit = e_aabbBit | e_centerOfMassBit | e_jointBit | e_pairBit | e_shapeBit;
    float scale = 1.f;
    int   layer = Renderer2D::DefaultPrimitiveLayer;

    void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        verts.clear();
        verts.reserve(vertexCount);

        for (int i = 0; i < vertexCount; i++)
        {
            Vector2f transformedVec = convVec(vertices[i]);
            verts.push_back(transformedVec);
        }

        Renderer2D::drawLines(verts, layer, convColor(color), 1.f, GLDrawMode::LineLoop);
    }

    void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override
    {
        verts.clear();
        verts.reserve(vertexCount);

        for (int i = 0; i < vertexCount; i++)
        {
            Vector2f transformedVec = convVec(vertices[i]);
            verts.push_back(transformedVec);
        }

        Renderer2D::drawLines(verts, layer, convColor(color), 1.f, GLDrawMode::TriangleFan);
    }

    void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override
    {
        Renderer2D::drawCircle(convVec(center), radius * scale, layer, convColor(color));
    }

    void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override
    {
        Renderer2D::drawCircle(convVec(center), radius * scale, layer, convColor(color), true);
    }

    void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override
    {
        Renderer2D::drawLine(convVec(p1), convVec(p2), layer + 1, convColor(color));
    }

    void DrawTransform(const b2Transform& xf) override
    { return; }

    void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override
    { return; };
};