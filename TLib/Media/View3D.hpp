
#pragma once

#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>

struct View3D
{
    enum class ViewMode
    {
        Perspective,
        Orthographic
    };

    float     znear    = 0.02f;
    float     zfar     = 500.f;
    float     fov      = 110.f;
    Vector3f  pos      ={ 0.0f, 0.0f, 0.0f };
    Quat      rot;
    ViewMode  viewmode = ViewMode::Perspective;

    inline void setPos(const Vector3f& posv) { this->pos ={ posv.x, posv.y, posv.z }; }
    [[nodiscard]] inline Vector3f getPos() const { return Vector3f(pos); }

    void lookAt(const Vector3f& target,
        const Vector3f& up    = Vector3f::up(),
        const Vector3f& altUp = Vector3f::backward())
    {
        rot = Quat::safeLookAt(pos, target, up, altUp);
    }

    [[nodiscard]]
    Mat4f getViewMatrix() const
    {
        Mat4f rotate = rot.toMat4f();
        Mat4f translate(1.0f);
        translate = translate.translated(-pos);
        return rotate * translate;
    }

    [[nodiscard]]
    Mat4f getPerspectiveMatrix(Vector2f size = Vector2f(Renderer::getFramebufferSize())) const
    {
        switch (viewmode)
        {
            case ViewMode::Perspective:
                return glm::perspective(glm::radians(fov), size.x / size.y, znear, zfar); break;
            case ViewMode::Orthographic:
            {
                Vector2f halfSize = size/2.f;
                return glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, znear, zfar); break;
            }
            default: return Mat4f(); break;
        }
    }
};
