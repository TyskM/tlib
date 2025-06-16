#pragma once

#include <TLib/Types/Types.hpp>
#include <TLib/Math.hpp>
#include <glm/ext.hpp>

struct View
{
    Vector2f center   = {0.f, 0.f};
    Vector2f size     = {1280, 720};
    Vector2f zoom     = {1.f, 1.f};
    float    rotation = 0.f; // Radians
    Rectf    viewport = {0.f, 0.f, 1.f, 1.f};

    void setBounds(const Rectf& rect)
    {
        center = rect.getPos() + rect.getSize() / 2.f;
        size = rect.getSize();
    }

    Vector2f topLeft() const
    { return center - (size / zoom / 2.f); }

    Vector2f bottomRight() const
    { center + (size / zoom / 2.f); }

    Rectf rect() const
    {
        return Rectf(topLeft(), Vector2f(size / zoom));
    }

    glm::mat4 getMatrix(bool flipY = false) const
    {
        Vector2f halfSize = size/2.f;
        const float l = center.x - halfSize.x;
        const float r = center.x + halfSize.x;
        float t;
        float b;

        if (flipY)
        {
            t = center.y - halfSize.y;
            b = center.y + halfSize.y;
        }
        else
        {
            t = center.y + halfSize.y;
            b = center.y - halfSize.y;
        }

        glm::mat4 mat(1.f);
        mat = glm::scale(mat, {zoom.x, zoom.y, 1.f});

        glm::mat4 orthoMat = glm::ortho(l, r, b, t, -1.f, 1.f);

        return mat * orthoMat;
    }
};

Recti getViewportSizePixels(const View& view, const Vector2f& targetSize)
{
    ASSERT(view.viewport.width && view.viewport.height);
    // Width and height must be >= 1

    Recti viewportPx{0, 0, 0, 0};
    if (view.viewport.x != 0) { viewportPx.x = targetSize.x * view.viewport.x; }
    if (view.viewport.y != 0) { viewportPx.y = targetSize.y * view.viewport.y; }
    viewportPx.width  = targetSize.x * view.viewport.width;
    viewportPx.height = targetSize.y * view.viewport.height;
    return viewportPx;
}

Vector2f localToWorldPoint(Vector2f localpos, const View& view, const Vector2f& targetSize)
{
    Recti viewportPx = getViewportSizePixels(view, targetSize);

    glm::mat4 mat = view.getMatrix();
    mat = glm::inverse(mat);

    // Viewport conversion first
    localpos.x = (localpos.x - viewportPx.x) / (viewportPx.width  / targetSize.x);
    localpos.y = (localpos.y - viewportPx.y) / (viewportPx.height / targetSize.y);

    // This was used to find the local y with bottomLeft as the viewport origin
    //localpos.y = (localpos.y + viewportPx.y - (targetSize.y - viewportPx.height)) / (viewportPx.height / targetSize.y);

    glm::vec3 ndc = glm::vec3(       localpos.x / targetSize.x,
    /* for top left origin: 1.f - */ localpos.y / targetSize.y, 0) * 2.f - 1.f;

    glm::vec4 worldPosition = mat * glm::vec4(ndc, 1);

    Vector2f worldPos{worldPosition.x, worldPosition.y};

    return worldPos;
}

Vector2f localToWorldPoint(Vector2f localpos, const View& view, const Vector2i& targetSize)
{ return localToWorldPoint(localpos, view, Vector2f(targetSize)); }

Vector2f worldToLocalPoint(Vector2f worldPos, const View& view, const Vector2f& targetSize)
{
    // DONE: I don't remember how to do any of this (I'm actually a genius nvm)
    // Thank you AndreiDespinoiu
    // https://old.reddit.com/r/opengl/comments/z3qt6h/whats_the_best_method_for_converting_3d_global/
    // TODO: This makes me realize localToWorldPoint could probably me simpler, maybe look into it.

    Recti viewportPx = getViewportSizePixels(view, targetSize);

    glm::mat4 mat = view.getMatrix();

    auto ndc = mat * glm::vec4(worldPos.x, worldPos.y, 0.f, 1.0f);
    glm::vec4 localPos = ndc;

    Vector2f ret(localPos.x, localPos.y);
    ret.x = math::convertRange(ret.x, -1.f, 1.f, 0.f, targetSize.x);
    ret.y = math::convertRange(ret.y, -1.f, 1.f, 0.f, targetSize.y);

    return ret;
}
