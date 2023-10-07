
#pragma once
#include <TLib/Logging.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Containers/Vector.hpp>
#include <box2d/box2d.h>

b2Vec2 tob2vec(const Vector2f& v)
{ return {v.x, v.y}; }

struct Intersection
{
    b2Fixture* fixture = nullptr;
    Vector2f   point;
    Vector2f   normal;
};

using RaycastFirstFilter = std::function<bool(b2Fixture&)>; // b2Fixture::GetUserData isn't const, so this can't be const either

namespace tlibdetail
{
    class RaycastFirstHandler : public b2RayCastCallback
    {
    public:
        bool       hit     = false;
        Vector2f   point;
        Vector2f   normal;
        b2Fixture* fixture = nullptr;
        RaycastFirstFilter filter = defaultFilter;

        static bool defaultFilter(b2Fixture&)
        { return true; }

        float ReportFixture(b2Fixture* _fixture, const b2Vec2& _point, const b2Vec2& _normal, float _fraction) override
        {
            if (filter(*_fixture))
            {
                hit     = true;
                point   = Vector2f(_point);
                normal  = Vector2f(_normal);
                fixture = _fixture;
                return _fraction;
            }
            return 1.f;
        }
    };

    class RaycastAllHandler : public b2RayCastCallback
    {
    public:
        Vector<Intersection>& outVec;

        RaycastAllHandler(Vector<Intersection>& outVec) : outVec{outVec} {}

        float ReportFixture(b2Fixture* _fixture, const b2Vec2& _point, const b2Vec2& _normal, float _fraction) override
        {
            outVec.emplace_back();
            auto& back   = outVec.back();
            back.fixture = _fixture;
            back.normal  = Vector2f(_normal);
            back.point   = Vector2f(_point);

            return 1.f;
        }
    };

    class AABBQueryAll : public b2QueryCallback
    {
    public:
        Vector<b2Body*>& outVec;

        AABBQueryAll(Vector<b2Body*>& outVec) : outVec{outVec} {}

        bool ReportFixture(b2Fixture* fixture) override
        {
            outVec.push_back(fixture->GetBody());
            return true;
        }
    };
}

struct RaycastFirst
{
    bool hit = false;
    Intersection intersec; // If hit is false, ignore
};

// Returns first fixture the raycast collides with. Ignores backfaces of fixtures
RaycastFirst raycastFirst(
    const b2World&            world,
    const Vector2f&           p1,
    const Vector2f&           p2,
    const RaycastFirstFilter& filter = tlibdetail::RaycastFirstHandler::defaultFilter)
{
    tlibdetail::RaycastFirstHandler rh;
    rh.filter = filter;
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

// Like raycastFirst, but casts a second ray backwards to find any shapes the first ray was inside.
RaycastFirst raycastFirstBackface(const b2World& world, const Vector2f& p1, const Vector2f& p2)
{
    Vector<Intersection> insecs;
    tlibdetail::RaycastAllHandler rh{insecs};
    world.RayCast(&rh, tob2vec(p1), tob2vec(p2));
    world.RayCast(&rh, tob2vec(p2), tob2vec(p1));

    RaycastFirst ret;
    if (insecs.empty())
    {
        ret.hit = false;
        ret.intersec.point = p2;
        return ret;
    }

    float dist = FLT_MAX;
    auto  it   = insecs.begin();
    for (auto& insec : insecs)
    {
        auto newDist = insec.point.distanceToSquared(p1);
        if (newDist < dist)
        {
            dist = newDist;
            it   = &insec;
        }
    }

    ret.hit      = true;
    ret.intersec = *it;
    return ret;
}

Vector<Intersection> raycastAll(const b2World& world, const Vector2f& p1, const Vector2f& p2)
{
    Vector<Intersection> ret;
    tlibdetail::RaycastAllHandler rh{ret};
    world.RayCast(&rh, tob2vec(p1), tob2vec(p2));
    return ret;
}

Vector<b2Body*> queryAABB(const b2World& world, const b2AABB& aabb)
{
    Vector<b2Body*> ret;
    tlibdetail::AABBQueryAll q(ret);
    world.QueryAABB(&q, aabb);
    return ret;
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