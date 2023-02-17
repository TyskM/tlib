#pragma once

#include <TLib/DataStructures.hpp>
#include <TLib/Math.hpp>
#include <glm/ext.hpp>

struct Camera2D
{
protected:
    Rectf bounds  = {0, 0, 6, 6};
    Vector2f zoom = {1.f, 1.f};
    float znear   = -1.f;
    float zfar    = 1.f;

public:
    Camera2D(float x, float y, float width, float height) : bounds{x, y, width, height} { }
    Camera2D() = default;

    [[nodiscard]]
    inline Rectf getBounds() const { return bounds; }
    inline void setBounds(const Rectf& b) { bounds = b; }

    [[nodiscard]]
    inline Vector2f getBoundsSize() const { return Vector2f(bounds.width, bounds.height); }
    inline void setBoundsSize(const Vector2f& size) { bounds.width = size.x; bounds.height = size.y; }

    [[nodiscard]]
    inline Vector2f getBoundsPos() const { return Vector2f(bounds.x, bounds.y); }
    inline void setBoundsPos(const Vector2f& pos) { bounds.x = pos.x; bounds.y = pos.y; }

    [[nodiscard]]
    inline Vector2f getZoom() const { return zoom; }
    inline void setZoom(const Vector2f& z) { zoom = z; }

    [[nodiscard]]
    inline float getNearClipDistance() const { return znear; }
    inline void setNearClipDistance(const float dist) { znear = dist; }

    [[nodiscard]]
    inline float getFarClipDistance() const { return zfar; }
    inline void setFarClipDistance(const float dist) { zfar = dist; }

    [[nodiscard]]
    Vector2f localToWorldCoords(Vector2f localpos)
    {
        glm::mat4 mat = getMatrix();
        mat = glm::inverse(mat);
        glm::vec3 ndc = glm::vec3(localpos.x / bounds.width, 1.0 - localpos.y / bounds.height, 0) * 2.f - 1.f;
        glm::vec4 worldPosition = mat * glm::vec4(ndc, 1);

        return { worldPosition.x, worldPosition.y };
    }

    [[nodiscard]]
    Vector2f localToWorldCoords(const Vector2i& localpos)
    { return localToWorldCoords(Vector2f(localpos)); }

    [[nodiscard]]
    glm::mat4 getMatrix() const
    {
        const float l = bounds.x;
        const float r = bounds.x + bounds.width;
        const float t = bounds.y;
        const float b = bounds.y + bounds.height;

        glm::mat4 mat(1.f);
        mat = glm::scale(mat, { zoom.x, zoom.y, 1.f });

        glm::mat4 orthoMat = glm::ortho(l, r, b, t, znear, zfar);
        
        return mat * orthoMat;
    }

	bool operator==(const Camera2D& other)
	{ return (bounds == other.bounds && zoom == other.zoom); }

	bool operator!=(const Camera2D& other)
	{ return !(operator==(other)); }
};
