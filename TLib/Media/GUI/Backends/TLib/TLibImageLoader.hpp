
#pragma once
#include <TLib/Media/GUI/BaseTypes.hpp>
#include <TLib/Media/GUI/ImageLoader.hpp>
#include <TLib/Media/GUI/Backends/TLib/TLibImage.hpp>

namespace agui
{
    class AGUI_BACKEND_DECLSPEC TLibImageLoader : public ImageLoader
    {
    public:
        TLibImageLoader(void) = default;

        virtual ~TLibImageLoader(void) { }

        virtual Image* loadImage(
            const String& fileName,
            bool          convertMask = false,
            bool          converToDisplayFormat = false)
        {
            return new TLibImage(fileName);
        }
    };

}