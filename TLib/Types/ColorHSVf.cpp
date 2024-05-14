
#include "ColorHSVf.hpp"
#include <TLib/Types/ColorRGBf.hpp>
#include <TLib/Convert.hpp>

ColorHSVf::ColorHSVf(const ColorRGBf& rgb)
{
    auto ret = rgbToHsv(rgb.r, rgb.g, rgb.b);
    h = ret[0];
    s = ret[1];
    v = ret[2];
}
