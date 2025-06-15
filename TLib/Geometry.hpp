
#pragma once

#include <TLib/Types/Types.hpp>
#include <TLib/Containers/Vector.hpp>

namespace Geom
{
    using Geometry2D = Vector<Vector2f>;

    struct SemiCircleParams
    {
        enum class Axis      : uint8_t { X, Y };
        enum class ExpandDir : uint8_t { Center, Clockwise, CounterClockwise };

        // Required
        Vector2f position;
        float    radius       = 10.f;
        float    arc          = 3.14;
        float    rotation     = 0.f;

        uint32_t   segmentCount = 16;
        Axis       axis         = Axis::Y; // Starting axis
        ExpandDir  expandDir    = ExpandDir::Center;
        bool       includeSides = true;

        SemiCircleParams() = default;
        SemiCircleParams(Vector2f position, float radius, float arc, float rotation, uint32_t segmentCount = 16) :
                         position{position}, radius{radius}, arc{arc}, rotation{rotation}, segmentCount{segmentCount} { }
    };

    static Geometry2D semiCircle(const SemiCircleParams& params)
    {
        const float maxArc = glm::pi<float>() * 2.f;

        float arc      = std::clamp(params.arc, -maxArc, maxArc);
        float rotation = params.rotation;

        if (params.axis == SemiCircleParams::Axis::Y)
        {
            rotation -= glm::pi<float>() / 2.f;
        }
        // Axis::X is the default

        switch (params.expandDir)
        {
            case SemiCircleParams::ExpandDir::Center:
                rotation -= arc / 2.f;
                break;

            case SemiCircleParams::ExpandDir::CounterClockwise:
                rotation -= arc;
                break;

         // case DrawSemiCircleParams::ExpandDir::Clockwise: Default
            default: break;
        }

        const float theta            = arc / static_cast<float>(params.segmentCount-1);
        const float tangetial_factor = tanf(theta);
        const float radial_factor    = cosf(theta);

        const float total = rotation + arc;
        Vector2f current = Vector2f(params.radius, 0).rotated(total);

        Vector<Vector2f> points;

        if (params.includeSides && arc < maxArc)
        {
            points.push_back(params.position);
        }

        for (uint32_t i = 0; i < params.segmentCount; i++)
        {
            points.push_back(Vector2f{ current.x + params.position.x, current.y + params.position.y });

            float tx = -current.y;
            float ty =  current.x;
            current.x += tx * tangetial_factor;
            current.y += ty * tangetial_factor;
            current.x *= radial_factor;
            current.y *= radial_factor;
        }

        return points;
    }
}