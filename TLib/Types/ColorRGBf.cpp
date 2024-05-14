
#include "ColorRGBf.hpp"

#include <TLib/Convert.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Types/ColorHSVf.hpp>

ColorRGBf::ColorRGBf(uint8_t rv, uint8_t gv, uint8_t bv) :
                   r{ uint8ToFloat(rv) },
                   g{ uint8ToFloat(gv) },
                   b{ uint8ToFloat(bv) } { }

ColorRGBf::ColorRGBf(const ColorRGBAf& rgba) :
                   r{ rgba.r },
                   g{ rgba.g },
                   b{ rgba.b } { }

ColorRGBf::ColorRGBf(const ColorHSVf& hsv)
{
    auto ret = hsvToRgb(hsv.h, hsv.s, hsv.v);
    r = ret[0];
    g = ret[1];
    b = ret[2];
}

ColorRGBf::operator Vector3f() const
{ return Vector3f(r, g, b); }
