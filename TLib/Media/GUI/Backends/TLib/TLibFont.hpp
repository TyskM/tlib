
#pragma once
#include <TLib/Media/GUI/BaseTypes.hpp>
#include <TLib/Media/Resource/Font.hpp>
#include <TLib/Media/Platform/Window.hpp>

namespace agui
{
    class AGUI_BACKEND_DECLSPEC TLibFont : public Font
    {
        ::Font*  font;
        bool     autoFree;
        int      characterSize;
        fs::path path;

    public:
        TLibFont(void) : font{nullptr}, autoFree{false}, characterSize{0} { }

        TLibFont(const String& fileName,
                 int           height,
                 FontFlags     fontFlags = FONT_DEFAULT_FLAGS) :
                 autoFree{true}, characterSize{height}, path{fileName}
        {
            font = new ::Font();
            if (!font->loadFromFile(fileName, height))
            {
                free();
                // TODO: use embedded fallback font here instead of assert
                RELASSERTMSGBOX(
                    false,
                    "Failed to load font",
                    fmt::format("Failed to load font from path: {}", fileName).c_str());
            }
        }

        virtual ~TLibFont(void)
        { if (autoFree) { free(); } }

        virtual void free()
        {
            if (font)
            { delete font; }
            font = nullptr;
            characterSize = 0;
        }

        ::Font* getFont() const
        { return font; }

        virtual int getLineHeight() const
        {
            if (font)
            { return font->newLineHeight(); }
            return 0;
        }

        virtual int getHeight() const
        { return characterSize; }

        void setHeight(int characterSize)
        { this->characterSize = characterSize; }

        virtual int getTextWidth(const String& text) const
        { return font->calcTextSize(text).x; }

        virtual const String& getPath() const
        { return path.string(); }

        virtual void setFont(::Font*      font,
                             const String& path,
                             int           characterSize,
                             bool          autoFree = false)
        {
            if (this->autoFree) { free(); }
            this->font          = font;
            this->characterSize = characterSize;
            this->autoFree      = autoFree;
            this->path          = path;
        }
        
        virtual void reload(const String& fileName,
            int           height,
            FontFlags     fontFlags   = FONT_DEFAULT_FLAGS,
            float         borderWidth = 0,
            agui::Color   borderColor = agui::Color())
        {
            if (autoFree) { free(); }

            this->autoFree      = true;
            this->characterSize = characterSize;

            font = new ::Font();
            if (!font->loadFromFile(fileName, height))
            {
                free();
                // TODO: use embedded fallback font here instead of assert
                RELASSERTMSGBOX(
                    false,
                    "Failed to load font",
                    fmt::format("Failed to load font from path: {}", fileName).c_str());
            }
        }
    };
}