
#pragma once

#include <TLib/Media/GUI/FontLoader.hpp>
#include <TLib/Media/GUI/Backends/TLib/TLibFont.hpp>

namespace agui
{
    class AGUI_BACKEND_DECLSPEC TLibFontLoader : public FontLoader
    {
    public:
        TLibFontLoader(void) { }

        virtual ~TLibFontLoader(void) { }

        virtual Font* loadFont(
            const String& fileName,
            int           height,
            FontFlags     fontFlags   = FONT_DEFAULT_FLAGS,
            float         borderWidth = 0,
            agui::Color   borderColor = agui::Color())
        {
            return new TLibFont(fileName, height, fontFlags);
        }
        
        virtual Font* loadEmptyFont()
        {
            return new TLibFont();
        }
    };

}