
#pragma once
#include <TLib/Media/GUI/BaseTypes.hpp>
#include <TLib/Media/Resource/Texture.hpp>

namespace agui
{
    class AGUI_BACKEND_DECLSPEC TLibImage : public Image
    {
        Texture* texturePtr;
        bool autoFree;

    public:
        TLibImage(void) : texturePtr{nullptr}, autoFree{false} { }

        TLibImage(const String& fileName)
        {
            texturePtr = new Texture();
            texturePtr->loadFromFile(fileName); // Will load fallback on failure
            texturePtr->setFilter(TextureFiltering::Linear);
        }

        virtual ~TLibImage(void)
        {
            if (autoFree) { free(); }
        }

        virtual int getWidth() const
        {
            if (texturePtr)
            { return texturePtr->getSize().x; }
            return 0;
        }

        virtual int getHeight() const
        {
            if (texturePtr)
            { return texturePtr->getSize().y; }
            return 0;
        }

        virtual Color getPixel(int x, int y) const
        {
            // unimplemented
            return agui::Color();
        }

        virtual void setPixel(int x, int y, const Color& color)
        {
            // unimplemented
        }

        virtual bool isAutoFreeing() const
        { return autoFree; }

        Texture* getBitmap() const
        { return texturePtr; }

        virtual void free()
        {
            if (texturePtr)
            { delete texturePtr; }
            texturePtr = nullptr;
        }
        
        virtual void setBitmap(Texture* bitmap, bool autoFree = false)
        {
            if (this->autoFree)
            { free(); }

            texturePtr = bitmap;
            this->autoFree = autoFree;
        }
        
    };
}