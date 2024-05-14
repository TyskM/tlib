
#pragma once
#include <TLib/DataStructures.hpp>

struct Transform3D
{
    Vector3f  pos;
    Vector3f  scale = { 1, 1, 1 };
    Quat      rot{ glm::vec3(0.f, 0.f, 0.f) };

    Transform3D() = default;
    Transform3D(const Vector3f& pos, const Vector3f& scale = Vector3f(1.f), const Quat& rot = Quat()) : pos{ pos }, scale{ scale }, rot{ rot } { }

    Mat4f getMatrix() const
    {
        // Scale, rotate, translate
        Mat4f mat(1.f);
        mat  = mat.scaled(scale);
        mat *= rot.toMat4f();
        mat  = mat.translated(pos);
        return mat;
    }

    void rotate(float angle, const Vector3f& axis)
    {
        rot *= Quat(angle, axis);
    }
};