
#pragma once

#include <TLib/Types/Vector3.hpp>
#include <TLib/Types/Mat4.hpp>
#include <glm/gtx/quaternion.hpp>

struct Quat
{
    float w = 1.f;
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;

    constexpr Quat() = default;
    constexpr Quat(float w, float x, float y, float z) : w{ w }, x{ x }, y{ y }, z{ z } { }
    constexpr Quat(const glm::quat& q) : w{ q.w }, x{ q.x }, y{ q.y }, z{ q.z } { }
              Quat(float angle, const Vector3f& axis) { *this = angleAxis(angle, axis); }

    glm::quat toGlm() const { return glm::quat(w, x, y, z); }

    String toString() const { return fmt::format("({}, {}, {}, {})", w, x, y, z); }

    static Quat angleAxis(float angle, const Vector3f& axis)
    { return glm::angleAxis(angle, glm::vec3(axis.x, axis.y, axis.z)); }

    Vector3f forward() const
    { return inverse().rotated(Vector3f::forward()); }

    Vector3f right() const
    { return inverse().rotated(Vector3f::right()); }

    Vector3f up() const
    { return inverse().rotated(Vector3f::up()); }

    Quat inverse() const
    { return glm::inverse(toGlm()); }

    Vector3f rotated(const Vector3f& axis) const
    { return Vector3f(glm::rotate(toGlm(), glm::vec3{ axis.x, axis.y, axis.z })); }

    Vector3f rotated(float axisx, float axisy, float axisz) const
    { return Vector3f(glm::rotate(toGlm(), glm::vec3{ axisx, axisy, axisz })); }

    Quat normalized() const
    { return glm::normalize(toGlm()); }

    Mat4f toMat4f() const
    { return glm::toMat4(toGlm()); }

    static Quat safeLookAt(const Vector3f& lookFrom,
        const Vector3f& lookTo,
        const Vector3f& up,
        const Vector3f& alternativeUp)
    {
        // https://stackoverflow.com/questions/18172388/glm-quaternion-lookat-function
        Vector3f  direction       = lookTo - lookFrom;
        float     directionLength = direction.length();

        // Check if the direction is valid; Also deals with NaN
        if (!(directionLength > 0.0001))
            return glm::quat(1, 0, 0, 0); // Just return identity

        direction = direction.normalized();

        // Is the normal up (nearly) parallel to direction?
        if (abs(direction.dot(up)) > .9999f)
        { return glm::quatLookAt(direction.toGlm(), alternativeUp.toGlm()); }
        else
        { return glm::quatLookAt(direction.toGlm(), up.toGlm()); }
    }

    Quat operator* (const Quat& other) { return toGlm() * other.toGlm(); }
    Quat operator*=(const Quat& other) { return toGlm() * other.toGlm(); }
};

static Vector3f operator* (const Vector3f& a, const Quat& b) { return Vector3f(a.toGlm() * b.toGlm()); }
static Vector3f operator*=(      Vector3f& a, const Quat& b) {    a = Vector3f(a.toGlm() * b.toGlm()); return a; }
