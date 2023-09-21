
#pragma once
#include <TLib/Media/GUI/Graphics.hpp>
#include <TLib/Media/GUI/Backends/TLib/TLibFont.hpp>
#include <TLib/Media/GUI/Backends/TLib/TLibImage.hpp>
#include <TLib/Media/Renderer2D.hpp>

#include <locale>
#include <codecvt>

namespace agui
{
    class AGUI_BACKEND_DECLSPEC TLibGraphics : public Graphics
    {
        std::wstring wide;
        Recti    clRct;
        int          height;
        Camera2D     origCamera;

        inline std::wstring StringToUnicode(const String& strIn)
        {
            wide.clear();
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            wide = converter.from_bytes(strIn);
            return wide;
        }

        ColorRGBAf colToTl(const agui::Color& c) const
        { return ColorRGBAf(c.getR(), c.getG(), c.getB(), c.getA()); }

    protected:
        void startClip()
        {
            int x = clRct.x;
            int y = clRct.y;
            int w = clRct.width;
            int h = clRct.height;
            // OpenGL's coords are from the bottom left
            // so we need to translate them here.
            y = height - (y + h);

            glEnable(GL_SCISSOR_TEST);
            glScissor(x, y, w, h);
        }

        void endClip()
        {
            glDisable(GL_SCISSOR_TEST);
        }
        
        virtual void setClippingRectangle(const Recti& rect)
        {
            clRct = rect;
            startClip();
        }

    public:
        TLibGraphics() = default;

        virtual ~TLibGraphics(void) { }

        virtual void _beginPaint()
        {
            origCamera = Renderer2D::getView();
            height = origCamera.getBoundsSize().y;

            Rectf vrect( Vector2f{0.f, 0.f}, origCamera.getBoundsSize() );
            Camera2D view;
            view.setBounds(vrect);
            Renderer2D::setView(view);

            auto size = origCamera.getBoundsSize();
            setClippingRectangle( Recti(0, 0, size.x, size.y) );
        }

        virtual void _endPaint()
        {
            Renderer2D::render();
            Renderer2D::setView(origCamera);
            endClip();
        }

        // TODO: This should be const
        virtual Vector2i getDisplaySize()
        {
            auto size = Renderer2D::getView().getBoundsSize();
            return Vector2i(size.x, size.y);
        }

        // TODO: This should be const
        virtual Recti getClippingRectangle()
        {
            return clRct;
        }

        virtual void drawImage(
            const Image*     bmp,
            const Vector2i&  position,
            const Vector2i&  regionStart,
            const Vector2i&  regionSize,
            const float&     opacity = 1.0f)
        {
            Texture* tex = reinterpret_cast<const TLibImage*>(bmp)->getBitmap();
            if (tex == nullptr) { return; }

            Rectf dstRect = {
                Vector2f(position.x + getOffset().x, position.y + getOffset().y),
                Vector2f(tex->getSize())
            };

            Rectf srcRect = {
                static_cast<float>(regionStart.x), static_cast<float>(regionStart.y),
                static_cast<float>(regionSize.x),  static_cast<float>(regionSize.y)
            };

            Renderer2D::drawTexture(*tex, srcRect, dstRect, 0, ColorRGBAf{ 1.f, 1.f, 1.f, opacity });
        }

        virtual void drawImage(
            const Image*    bmp,
            const Vector2i& position,
            const float&    opacity = 1.0f)
        {
            Texture* tex = reinterpret_cast<const TLibImage*>(bmp)->getBitmap();
            if (tex == nullptr) { return; }

            Rectf dstRect = {
                Vector2f(position.x + getOffset().x, position.y + getOffset().y),
                Vector2f(tex->getSize())
            };

            Renderer2D::drawTexture(*tex, dstRect, 0, ColorRGBAf{1.f, 1.f, 1.f, opacity});
        }

        virtual void drawScaledImage(
            const Image*     bmp,
            const Vector2i&     position,
            const Vector2i&     regionStart,
            const Vector2i& regionScale,
            const Vector2i& scale,
            const float&     opacity = 1.0f)
        {
            // TODO: implement
        }

        virtual void drawText(
            const Vector2i&  position,
            const char*      text,
            const Color&     color,
            const Font*      font,
            AlignmentEnum    align = ALIGN_LEFT)
        {
            TLibFont* tlFont = (TLibFont*)(font);
            SDFFont* realFont = tlFont->getFont();

            Vector2f pos =
               { (float)(position.x + getOffset().x),
                 (float)(position.y + getOffset().y) };
            Renderer2D::drawCircle(Vector2f(pos), 3.f, 1, ColorRGBAf::red(), true, 12);
            pos.y -= font->getLineHeight() / 4; // TODO: fix ur friggin font rendering bro

            // TODO: handle ALIGN_CENTER && ALIGN_RIGHT
            Renderer2D::drawText(text, *realFont, pos, 2, colToTl(color));
        }

        virtual void drawRectangle(
            const Recti& rect,
            const Color& color)
        {
            Rectf tlrect = {
                static_cast<float>(rect.x) + static_cast<float>(getOffset().x),
                static_cast<float>(rect.y) + static_cast<float>(getOffset().y),
                static_cast<float>(rect.width),
                static_cast<float>(rect.height)
            };

            Renderer2D::drawRect(tlrect, 1, colToTl(color), false);
        }

        virtual void drawFilledRectangle(
            const Recti& rect,
            const Color&     color)
        {
            Rectf tlrect = {
                static_cast<float>(rect.x)  + static_cast<float>(getOffset().x),
                static_cast<float>(rect.y)   + static_cast<float>(getOffset().y),
                static_cast<float>(rect.width),
                static_cast<float>(rect.height)
            };

            Renderer2D::drawRect(tlrect, 1, colToTl(color), true);
        }
        
        virtual void drawPixel(
            const Vector2i& point,
            const Color& color)
        {
            // TODO: figure out why tf we need to be drawing pixels
        }
        
        virtual void drawCircle(
            const Vector2i& center,
            float           radius,
            const Color&    color)
        {
            Vector2f realPoint = {
                static_cast<float>(center.x) + static_cast<float>(getOffset().x),
                static_cast<float>(center.y) + static_cast<float>(getOffset().y)
            };

            Renderer2D::drawCircle(realPoint, radius, 1, colToTl(color), false);
        }
        
        virtual void drawFilledCircle(
            const Vector2i& center,
            float           radius,
            const Color&    color)
        {
            Vector2f realPoint = {
                static_cast<float>(center.x) + static_cast<float>(getOffset().x),
                static_cast<float>(center.y) + static_cast<float>(getOffset().y)
            };

            Renderer2D::drawCircle(realPoint, radius, 1, colToTl(color), true);
        }
        
        virtual void drawLine(
            const Vector2i& start,
            const Vector2i& end,
            const Color& color)
        {
            Renderer2D::drawLine(
                Vector2f( start.x + getOffset().x, start.y + getOffset().y ),
                Vector2f(   end.x + getOffset().x,   end.y + getOffset().y ),
                1, colToTl(color));
        }

        virtual void setTargetImage(const Image* target)
        {
            // TODO: unimplemented, find out what this does
        }
        
        virtual void resetTargetImage()
        {
            // TODO: unimplemented, find out what this does
        }
        
    };
}