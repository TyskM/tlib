
#pragma once

#include <TLib/String.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Convert.hpp>
#include <TLib/Types/Vector4.hpp>
#include <TLib/Types/ColorRGBAi.hpp>
#include <TLib/Types/ColorRGBf.hpp>

// Represents a RGBA color with values 0f-1f
struct ColorRGBAf
{
    float r = 0;
    float g = 0;
    float b = 0;
    float a = 1.f;

    constexpr ColorRGBAf() = default;

    ColorRGBAf(String hex)
    {
        if (hex.at(0) == '#') { hex.erase(0, 1); }
        ASSERT( hex.size() == (6) ); // Invalid Hex code

        int ir, ig, ib;
        sscanf(hex.c_str(), "%02x%02x%02x", &ir, &ig, &ib);
        r = ir/255.f;
        g = ig/255.f;
        b = ib/255.f;
    }

    ColorRGBAf(const char* hex) : ColorRGBAf(String(hex)) { }

    constexpr ColorRGBAf(float r, float g, float b, float a = 1.f) :
                            r{ r },  g{ g },  b{ b },  a{ a } { }

    explicit constexpr ColorRGBAf(int r, int g, int b, int a = 255) :
                                r{ uint8ToFloat(r) }, g{ uint8ToFloat(g) },
                                b{ uint8ToFloat(b) }, a{ uint8ToFloat(a) } { }

    constexpr ColorRGBAf(const ColorRGBAi& v) : ColorRGBAf(v.r, v.g, v.b, v.a) { }
    constexpr ColorRGBAi toRGBAi()  const { return ColorRGBAi(floatToUint8(r), floatToUint8(g), floatToUint8(b), floatToUint8(a)); }
    constexpr operator ColorRGBAi() const { return toRGBAi(); }

    explicit constexpr ColorRGBAf(const ColorRGBf& v) : r{v.r}, g{v.g}, b{v.b} { }
    constexpr ColorRGBf toRGBf()            const { return ColorRGBf(r, g, b); }
    explicit constexpr operator ColorRGBf() const { return toRGBf(); }

    ColorRGBAf& setR(float r) { this->r = r; return *this; }
    ColorRGBAf& setG(float g) { this->g = g; return *this; }
    ColorRGBAf& setB(float b) { this->b = b; return *this; }
    ColorRGBAf& setA(float a) { this->a = a; return *this; }

    constexpr Vector4f toVec4()   const { return Vector4f(r, g, b, a); }
    constexpr operator Vector4f() const { return toVec4(); }

    String toString() const { return toVec4().toString(); }
    operator String() const { return toString(); }




    bool operator==(const ColorRGBAf& other) { return r == other.r && g == other.g && b == other.b && a == other.a; }
    bool operator!=(const ColorRGBAf& other) { return !(operator==(other)); }

    static constexpr inline ColorRGBAf red()         { return ColorRGBAf{ 255, 0,   0    }; }
    static constexpr inline ColorRGBAf green()       { return ColorRGBAf{ 0,   255, 0    }; }
    static constexpr inline ColorRGBAf blue()        { return ColorRGBAf{ 0,   0,   255  }; }
    static constexpr inline ColorRGBAf white()       { return ColorRGBAf{ 255, 255, 255  }; }
    static constexpr inline ColorRGBAf black()       { return ColorRGBAf{ 0,   0,   0    }; }
    static constexpr inline ColorRGBAf transparent() { return ColorRGBAf{ 0,   0,   0, 0 }; }
    static constexpr inline ColorRGBAf purple()      { return ColorRGBAf{ 127, 0,   255  }; }
    static constexpr inline ColorRGBAf yellow()      { return ColorRGBAf{ 255, 255, 0    }; }
    static constexpr inline ColorRGBAf orange()      { return ColorRGBAf{ 255, 128, 0    }; }
    static constexpr inline ColorRGBAf magenta()     { return ColorRGBAf{ 255, 0,   255  }; }
    static constexpr inline ColorRGBAf darkMagenta() { return ColorRGBAf{ 139, 0,   139  }; }
    static constexpr inline ColorRGBAf lime()        { return ColorRGBAf{ 0,   255, 0    }; }
    static constexpr inline ColorRGBAf cyan()        { return ColorRGBAf{ 0,   255, 255  }; }
    static constexpr inline ColorRGBAf steelBlue()   { return ColorRGBAf{ 70,  130, 180  }; }
    static constexpr inline ColorRGBAf royalBlue()   { return ColorRGBAf{ 65,  105, 225  }; }
    static constexpr inline ColorRGBAf gold()        { return ColorRGBAf{ 255, 215, 0    }; }

    static constexpr inline ColorRGBAf maroon                   () { return ColorRGBAf(128,0,0)     ; };
    static constexpr inline ColorRGBAf darkRed                  () { return ColorRGBAf(139,0,0)     ; };
    static constexpr inline ColorRGBAf brown                    () { return ColorRGBAf(165,42,42)   ; };
    static constexpr inline ColorRGBAf firebrick                () { return ColorRGBAf(178,34,34)   ; };
    static constexpr inline ColorRGBAf crimson                  () { return ColorRGBAf(220,20,60)   ; };
    static constexpr inline ColorRGBAf tomato                   () { return ColorRGBAf(255,99,71)   ; };
    static constexpr inline ColorRGBAf coral                    () { return ColorRGBAf(255,127,80)  ; };
    static constexpr inline ColorRGBAf indianRed                () { return ColorRGBAf(205,92,92)   ; };
    static constexpr inline ColorRGBAf lightCoral               () { return ColorRGBAf(240,128,128) ; };
    static constexpr inline ColorRGBAf darkSalmon               () { return ColorRGBAf(233,150,122) ; };
    static constexpr inline ColorRGBAf salmon                   () { return ColorRGBAf(250,128,114) ; };
    static constexpr inline ColorRGBAf lightSalmon              () { return ColorRGBAf(255,160,122) ; };
    static constexpr inline ColorRGBAf orangeRed                () { return ColorRGBAf(255,69,0)    ; };
    static constexpr inline ColorRGBAf darkOrange               () { return ColorRGBAf(255,140,0)   ; };
    static constexpr inline ColorRGBAf darkGoldenRod            () { return ColorRGBAf(184,134,11)  ; };
    static constexpr inline ColorRGBAf goldenRod                () { return ColorRGBAf(218,165,32)  ; };
    static constexpr inline ColorRGBAf paleGoldenRod            () { return ColorRGBAf(238,232,170) ; };
    static constexpr inline ColorRGBAf darkKhaki                () { return ColorRGBAf(189,183,107) ; };
    static constexpr inline ColorRGBAf khaki                    () { return ColorRGBAf(240,230,140) ; };
    static constexpr inline ColorRGBAf olive                    () { return ColorRGBAf(128,128,0)   ; };
    static constexpr inline ColorRGBAf yellowGreen              () { return ColorRGBAf(154,205,50)  ; };
    static constexpr inline ColorRGBAf darkOliveGreen           () { return ColorRGBAf(85,107,47)   ; };
    static constexpr inline ColorRGBAf oliveDrab                () { return ColorRGBAf(107,142,35)  ; };
    static constexpr inline ColorRGBAf lawnGreen                () { return ColorRGBAf(124,252,0)   ; };
    static constexpr inline ColorRGBAf chartreuse               () { return ColorRGBAf(127,255,0)   ; };
    static constexpr inline ColorRGBAf greenYellow              () { return ColorRGBAf(173,255,47)  ; };
    static constexpr inline ColorRGBAf darkGreen                () { return ColorRGBAf(0,100,0)     ; };
    static constexpr inline ColorRGBAf forestGreen              () { return ColorRGBAf(34,139,34)   ; };
    static constexpr inline ColorRGBAf limeGreen                () { return ColorRGBAf(50,205,50)   ; };
    static constexpr inline ColorRGBAf lightGreen               () { return ColorRGBAf(144,238,144) ; };
    static constexpr inline ColorRGBAf paleGreen                () { return ColorRGBAf(152,251,152) ; };
    static constexpr inline ColorRGBAf darkSeaGreen             () { return ColorRGBAf(143,188,143) ; };
    static constexpr inline ColorRGBAf mediumSpringGreen        () { return ColorRGBAf(0,250,154)   ; };
    static constexpr inline ColorRGBAf springGreen              () { return ColorRGBAf(0,255,127)   ; };
    static constexpr inline ColorRGBAf seaGreen                 () { return ColorRGBAf(46,139,87)   ; };
    static constexpr inline ColorRGBAf mediumAquaMarine         () { return ColorRGBAf(102,205,170) ; };
    static constexpr inline ColorRGBAf mediumSeaGreen           () { return ColorRGBAf(60,179,113)  ; };
    static constexpr inline ColorRGBAf lightSeaGreen            () { return ColorRGBAf(32,178,170)  ; };
    static constexpr inline ColorRGBAf darkSlateGray            () { return ColorRGBAf(47,79,79)    ; };
    static constexpr inline ColorRGBAf teal                     () { return ColorRGBAf(0,128,128)   ; };
    static constexpr inline ColorRGBAf darkCyan                 () { return ColorRGBAf(0,139,139)   ; };
    static constexpr inline ColorRGBAf aqua                     () { return ColorRGBAf(0,255,255)   ; };
    static constexpr inline ColorRGBAf lightCyan                () { return ColorRGBAf(224,255,255) ; };
    static constexpr inline ColorRGBAf darkTurquoise            () { return ColorRGBAf(0,206,209)   ; };
    static constexpr inline ColorRGBAf turquoise                () { return ColorRGBAf(64,224,208)  ; };
    static constexpr inline ColorRGBAf mediumTurquoise          () { return ColorRGBAf(72,209,204)  ; };
    static constexpr inline ColorRGBAf paleTurquoise            () { return ColorRGBAf(175,238,238) ; };
    static constexpr inline ColorRGBAf aquaMarine               () { return ColorRGBAf(127,255,212) ; };
    static constexpr inline ColorRGBAf powderBlue               () { return ColorRGBAf(176,224,230) ; };
    static constexpr inline ColorRGBAf cadetBlue                () { return ColorRGBAf(95,158,160)  ; };
    static constexpr inline ColorRGBAf cornFlowerBlue           () { return ColorRGBAf(100,149,237) ; };
    static constexpr inline ColorRGBAf deepSkyBlue              () { return ColorRGBAf(0,191,255)   ; };
    static constexpr inline ColorRGBAf dodgerBlue               () { return ColorRGBAf(30,144,255)  ; };
    static constexpr inline ColorRGBAf lightBlue                () { return ColorRGBAf(173,216,230) ; };
    static constexpr inline ColorRGBAf skyBlue                  () { return ColorRGBAf(135,206,235) ; };
    static constexpr inline ColorRGBAf lightSkyBlue             () { return ColorRGBAf(135,206,250) ; };
    static constexpr inline ColorRGBAf midnightBlue             () { return ColorRGBAf(25,25,112)   ; };
    static constexpr inline ColorRGBAf navy                     () { return ColorRGBAf(0,0,128)     ; };
    static constexpr inline ColorRGBAf darkBlue                 () { return ColorRGBAf(0,0,139)     ; };
    static constexpr inline ColorRGBAf mediumBlue               () { return ColorRGBAf(0,0,205)     ; };
    static constexpr inline ColorRGBAf blueViolet               () { return ColorRGBAf(138,43,226)  ; };
    static constexpr inline ColorRGBAf indigo                   () { return ColorRGBAf(75,0,130)    ; };
    static constexpr inline ColorRGBAf darkSlateBlue            () { return ColorRGBAf(72,61,139)   ; };
    static constexpr inline ColorRGBAf slateBlue                () { return ColorRGBAf(106,90,205)  ; };
    static constexpr inline ColorRGBAf mediumSlateBlue          () { return ColorRGBAf(123,104,238) ; };
    static constexpr inline ColorRGBAf mediumPurple             () { return ColorRGBAf(147,112,219) ; };
    static constexpr inline ColorRGBAf darkViolet               () { return ColorRGBAf(148,0,211)   ; };
    static constexpr inline ColorRGBAf darkOrchid               () { return ColorRGBAf(153,50,204)  ; };
    static constexpr inline ColorRGBAf mediumOrchid             () { return ColorRGBAf(186,85,211)  ; };
    static constexpr inline ColorRGBAf thistle                  () { return ColorRGBAf(216,191,216) ; };
    static constexpr inline ColorRGBAf plum                     () { return ColorRGBAf(221,160,221) ; };
    static constexpr inline ColorRGBAf violet                   () { return ColorRGBAf(238,130,238) ; };
    static constexpr inline ColorRGBAf orchid                   () { return ColorRGBAf(218,112,214) ; };
    static constexpr inline ColorRGBAf mediumVioletRed          () { return ColorRGBAf(199,21,133)  ; };
    static constexpr inline ColorRGBAf paleVioletRed            () { return ColorRGBAf(219,112,147) ; };
    static constexpr inline ColorRGBAf deepPink                 () { return ColorRGBAf(255,20,147)  ; };
    static constexpr inline ColorRGBAf hotPink                  () { return ColorRGBAf(255,105,180) ; };
    static constexpr inline ColorRGBAf lightPink                () { return ColorRGBAf(255,182,193) ; };
    static constexpr inline ColorRGBAf pink                     () { return ColorRGBAf(255,192,203) ; };
    static constexpr inline ColorRGBAf antiqueWhite             () { return ColorRGBAf(250,235,215) ; };
    static constexpr inline ColorRGBAf beige                    () { return ColorRGBAf(245,245,220) ; };
    static constexpr inline ColorRGBAf bisque                   () { return ColorRGBAf(255,228,196) ; };
    static constexpr inline ColorRGBAf blanchedAlmond           () { return ColorRGBAf(255,235,205) ; };
    static constexpr inline ColorRGBAf wheat                    () { return ColorRGBAf(245,222,179) ; };
    static constexpr inline ColorRGBAf cornSilk                 () { return ColorRGBAf(255,248,220) ; };
    static constexpr inline ColorRGBAf lemonChiffon             () { return ColorRGBAf(255,250,205) ; };
    static constexpr inline ColorRGBAf lightYellow              () { return ColorRGBAf(255,255,224) ; };
    static constexpr inline ColorRGBAf saddleBrown              () { return ColorRGBAf(139,69,19)   ; };
    static constexpr inline ColorRGBAf sienna                   () { return ColorRGBAf(160,82,45)   ; };
    static constexpr inline ColorRGBAf chocolate                () { return ColorRGBAf(210,105,30)  ; };
    static constexpr inline ColorRGBAf peru                     () { return ColorRGBAf(205,133,63)  ; };
    static constexpr inline ColorRGBAf sandyBrown               () { return ColorRGBAf(244,164,96)  ; };
    static constexpr inline ColorRGBAf burlyWood                () { return ColorRGBAf(222,184,135) ; };
    static constexpr inline ColorRGBAf tan                      () { return ColorRGBAf(210,180,140) ; };
    static constexpr inline ColorRGBAf rosyBrown                () { return ColorRGBAf(188,143,143) ; };
    static constexpr inline ColorRGBAf moccasin                 () { return ColorRGBAf(255,228,181) ; };
    static constexpr inline ColorRGBAf navajoWhite              () { return ColorRGBAf(255,222,173) ; };
    static constexpr inline ColorRGBAf peachPuff                () { return ColorRGBAf(255,218,185) ; };
    static constexpr inline ColorRGBAf mistyRose                () { return ColorRGBAf(255,228,225) ; };
    static constexpr inline ColorRGBAf lavenderBlush            () { return ColorRGBAf(255,240,245) ; };
    static constexpr inline ColorRGBAf linen                    () { return ColorRGBAf(250,240,230) ; };
    static constexpr inline ColorRGBAf oldLace                  () { return ColorRGBAf(253,245,230) ; };
    static constexpr inline ColorRGBAf papayaWhip               () { return ColorRGBAf(255,239,213) ; };
    static constexpr inline ColorRGBAf seaShell                 () { return ColorRGBAf(255,245,238) ; };
    static constexpr inline ColorRGBAf mintCream                () { return ColorRGBAf(245,255,250) ; };
    static constexpr inline ColorRGBAf slateGray                () { return ColorRGBAf(112,128,144) ; };
    static constexpr inline ColorRGBAf lightSlateGray           () { return ColorRGBAf(119,136,153) ; };
    static constexpr inline ColorRGBAf lightSteelBlue           () { return ColorRGBAf(176,196,222) ; };
    static constexpr inline ColorRGBAf lavender                 () { return ColorRGBAf(230,230,250) ; };
    static constexpr inline ColorRGBAf floralWhite              () { return ColorRGBAf(255,250,240) ; };
    static constexpr inline ColorRGBAf aliceBlue                () { return ColorRGBAf(240,248,255) ; };
    static constexpr inline ColorRGBAf ghostWhite               () { return ColorRGBAf(248,248,255) ; };
    static constexpr inline ColorRGBAf honeydew                 () { return ColorRGBAf(240,255,240) ; };
    static constexpr inline ColorRGBAf ivory                    () { return ColorRGBAf(255,255,240) ; };
    static constexpr inline ColorRGBAf azure                    () { return ColorRGBAf(240,255,255) ; };
    static constexpr inline ColorRGBAf snow                     () { return ColorRGBAf(255,250,250) ; };
    static constexpr inline ColorRGBAf dimGray                  () { return ColorRGBAf(105,105,105) ; };
    static constexpr inline ColorRGBAf gray                     () { return ColorRGBAf(128,128,128) ; };
    static constexpr inline ColorRGBAf darkGray                 () { return ColorRGBAf(169,169,169) ; };
    static constexpr inline ColorRGBAf silver                   () { return ColorRGBAf(192,192,192) ; };
    static constexpr inline ColorRGBAf lightGray                () { return ColorRGBAf(211,211,211) ; };
    static constexpr inline ColorRGBAf whiteSmoke               () { return ColorRGBAf(245,245,245) ; };
};
