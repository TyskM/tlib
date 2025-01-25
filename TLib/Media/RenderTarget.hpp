
#pragma once
#include <TLib/Media/GL/FrameBuffer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/Resource/Texture.hpp>

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

    glm::vec3 ndc = glm::vec3(
              localpos.x / targetSize.x,
        /*1.f - for top left origin*/
              localpos.y / targetSize.y, 0)
              * 2.f - 1.f;

    glm::vec4 worldPosition = mat * glm::vec4(ndc, 1);

    Vector2f worldPos{worldPosition.x, worldPosition.y};

    return worldPos;
}

Vector2f localToWorldPoint(Vector2f localpos, const View& view, const Vector2i& targetSize)
{ return localToWorldPoint(localpos, view, Vector2f(targetSize)); }

struct RenderTarget
{
    static inline RenderTarget* boundRenderTarget = nullptr;

    DISABLE_COPY(RenderTarget);
    DISABLE_MOVE(RenderTarget);
    RenderTarget() = default;

    FrameBuffer fbo;
    Texture     texture;
    View        view;

    void create()
    {
        if (created()) { return; }
        fbo.create();
        texture.create();
        fbo.setTexture(texture);
    }

    bool created() const
    { return fbo.created(); }

    void setSize(int width, int height)
    { texture.setData(NULL, width, height); }

    void setSize(const Vector2i& v)
    { setSize(v.x, v.y); }

    Vector2i getSize() const
    { return texture.getSize(); }

    void bind()
    {
        fbo.bind();
        boundRenderTarget = this;
    }

    static void unbind()
    {
        FrameBuffer::unbind();
        boundRenderTarget = nullptr;
    }

    static RenderTarget* getBoundRenderTarget()
    { return RenderTarget::boundRenderTarget; }

    Recti getViewportSizePixels(const View& view) const
    { return ::getViewportSizePixels(view, Vector2f(getSize())); }

    Recti getViewportSizePixels() const
    { return getViewportSizePixels(view); }

    Vector2f localToWorldPoint(Vector2f localpos) const
    { return localToWorldPoint(localpos, view); }

    Vector2f localToWorldPoint(Vector2f localpos, const View& _view) const
    { return ::localToWorldPoint(localpos, _view, Vector2f(getSize())); }

    Vector2f worldToLocalPoint(Vector2f worldpos) const
    {
        ASSERT(false); // NOT IMPLEMENTED
        return Vector2f();
    }
};
