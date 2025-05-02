
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
        return ::worldToLocalPoint(worldpos, view, Vector2f(getSize()));
    }
};
