
#pragma once
#include <TLib/Media/GUI/Graphics.hpp>
#include <TLib/Media/GUI/Backends/TLib/TLibFont.hpp>
#include <TLib/Media/Renderer2D.hpp>

#include <locale>
#include <codecvt>

namespace agui
{
    class AGUI_BACKEND_DECLSPEC TLibGraphics : public Graphics
    {
        std::wstring wide;
        Rectangle    clRct;
        int          height;
        Camera2D     origCamera;

        inline std::wstring StringToUnicode(const std::string& strIn)
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
            int x = clRct.getX();
            int y = clRct.getY();
            int w = clRct.getWidth();
            int h = clRct.getHeight();
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
        
        virtual void setClippingRectangle(const Rectangle& rect)
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
            setClippingRectangle( Rectangle(0, 0, size.x, size.y) );
        }

        virtual void _endPaint()
        {
            Renderer2D::setView(origCamera);
            endClip();
        }

        // TODO: This should be const
        virtual Dimension getDisplaySize()
        {
            auto size = Renderer2D::getView().getBoundsSize();
            return agui::Dimension(size.x, size.y);
        }

        // TODO: This should be const
        virtual Rectangle getClippingRectangle()
        {
            return clRct;
        }

        virtual void drawImage(
            const Image*     bmp,
            const Point&     position,
            const Point&     regionStart,
            const Dimension& regionSize,
            const float&     opacity = 1.0f)
        {
            // TODO: implement
            //Renderer2D::drawTexture()
        }

        virtual void drawImage(
            const Image* bmp,
            const Point& position,
            const float& opacity = 1.0f)
        {
            // TODO: implement
        }

        virtual void drawScaledImage(
            const Image*     bmp,
            const Point&     position,
            const Point&     regionStart,
            const Dimension& regionScale,
            const Dimension& scale,
            const float&     opacity = 1.0f)
        {
            // TODO: implement
        }

        virtual void drawText(
            const Point&  position,
            const char*   text,
            const Color&  color,
            const Font*   font,
            AlignmentEnum align = ALIGN_LEFT)
        {
            TLibFont* tlFont = (TLibFont*)(font);
            SDFFont* realFont = tlFont->getFont();

            Vector2f pos =
               { (float)(position.getX() + getOffset().getX()),
                 (float)(position.getY() + getOffset().getY()) };

            // TODO: handle ALIGN_CENTER && ALIGN_RIGHT
            Renderer2D::drawText(text, *realFont, pos, 2, colToTl(color));
        }

        virtual void drawRectangle(
            const Rectangle& rect,
            const Color&     color)
        {
            Rectf tlrect = {
                static_cast<float>(rect.getLeft())  + static_cast<float>(getOffset().getX()),
                static_cast<float>(rect.getTop())   + static_cast<float>(getOffset().getY()),
                static_cast<float>(rect.getWidth()),
                static_cast<float>(rect.getHeight())
            };

            Renderer2D::drawRect(tlrect, 1, colToTl(color), false);
        }

        virtual void drawFilledRectangle(
            const Rectangle& rect,
            const Color&     color)
        {
            Rectf tlrect = {
                static_cast<float>(rect.getLeft())  + static_cast<float>(getOffset().getX()),
                static_cast<float>(rect.getTop())   + static_cast<float>(getOffset().getY()),
                static_cast<float>(rect.getWidth()),
                static_cast<float>(rect.getHeight())
            };

            Renderer2D::drawRect(tlrect, 1, colToTl(color), true);
        }
        
        virtual void drawPixel(
            const Point& point,
            const Color& color)
        {
            // TODO: figure out why tf we need to be drawing pixels
        }
        
        virtual void drawCircle(
            const Point& center,
            float        radius,
            const Color& color)
        {
            Vector2f realPoint = {
                static_cast<float>(center.getX()) + static_cast<float>(getOffset().getX()),
                static_cast<float>(center.getY()) + static_cast<float>(getOffset().getY())
            };

            Renderer2D::drawCircle(realPoint, radius, 1, colToTl(color), false);
        }
        
        virtual void drawFilledCircle(
            const Point& center,
            float        radius,
            const Color& color)
        {
            Vector2f realPoint = {
                static_cast<float>(center.getX()) + static_cast<float>(getOffset().getX()),
                static_cast<float>(center.getY()) + static_cast<float>(getOffset().getY())
            };

            Renderer2D::drawCircle(realPoint, radius, 1, colToTl(color), true);
        }
        
        virtual void drawLine(
            const Point& start,
            const Point& end,
            const Color& color)
        {
            Renderer2D::drawLine(
                Vector2f( start.getX() + getOffset().getX(), start.getY() + getOffset().getY() ),
                Vector2f(   end.getX() + getOffset().getX(),   end.getY() + getOffset().getY() ),
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