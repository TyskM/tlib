
#pragma once

#include <TLib/String.hpp>
#include <TLib/Logging.hpp>
#include <TLib/Types/Vector3.hpp>
#include <TLib/Types/Vector4.hpp>

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Mat4f
{
    glm::mat4 matrix;

    Mat4f() = default;
    Mat4f(float x) { matrix = glm::mat4(x); }
    Mat4f(const glm::mat4& m) : matrix{ m } { }

    String toString() const
    {
        return fmt::format(fmt::runtime("[{}][{}][{}][{}]\n[{}][{}][{}][{}]\n[{}][{}][{}][{}]\n[{}][{}][{}][{}]"),
            matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
            matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
            matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
            matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
    }
    operator String() const
    { return toString(); }

    const float* data() const
    { return glm::value_ptr(matrix); }

    Mat4f translated(const Vector3f& transform) const
    { return glm::translate(matrix, transform.toGlm()); }

    Mat4f translated(const Vector4f& transform) const
    { return glm::translate(matrix, glm::vec3(transform.toGlm())); }

    Mat4f scaled(const Vector3f& _scale) const
    { return glm::scale(matrix, glm::vec3(_scale.x, _scale.y, _scale.z)); }

    Mat4f scaled(float x, float y, float z) const
    { return glm::scale(matrix, glm::vec3(x, y, z)); }

    Mat4f inverse() const
    { return glm::inverse(toGlm()); }

    glm::mat4 toGlm() const { return matrix; }

    Mat4f operator* (const Mat4f& other) const { return matrix * other.matrix; }
    Mat4f operator*=(const Mat4f& other)       { matrix *= other.matrix; return *this; }

    auto operator[](int32_t index) { return matrix[index]; }
};

static Vector3f operator* (const Vector3f& a, const Mat4f& b) { return Vector3f(glm::vec4(a.x, a.y, a.z, 1.f) * b.toGlm()); }
static Vector3f operator*=(      Vector3f& a, const Mat4f& b) {    a = Vector3f(glm::vec4(a.x, a.y, a.z, 1.f) * b.toGlm()); return a; }

static Vector4f operator* (const Vector4f& a, const Mat4f& b) { return a.toGlm() * b.toGlm(); }
static Vector4f operator*=(      Vector4f& a, const Mat4f& b) {    a = a.toGlm() * b.toGlm(); return a; }

static Vector4f operator* (const Mat4f& m, const Vector4f& v) { return m.toGlm() * v.toGlm(); }