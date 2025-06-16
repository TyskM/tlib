
#pragma once
#include <TLib/Media/GL/FrameBuffer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/Resource/Texture.hpp>

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
