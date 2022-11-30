#pragma once

#include "../thirdparty/SDL_FontCache.h"
#include <SDL2/SDL_ttf.h>
#include "../Macros.hpp"
#include "../DataStructures.hpp"
#include "../NonAssignable.hpp"
#include <string>

enum class FontStyle
{
    NORMAL        = 0,
    BOLD          = 1,
    ITALIC        = 2,
    UNDERLINE     = 4,
    STRIKETHROUGH = 8
};

struct Font : NonAssignable
{
    FC_Font* _font = nullptr;

    Font() = default;
    Font(SDL_Renderer* renderer, const std::string& fontpath, ColorRGBAi color = { 0, 0, 0, 255 }, int ptsize = 30, FontStyle style = FontStyle::NORMAL)
    {
        loadFromPath(renderer, fontpath, color, ptsize);
    }

    ~Font()
    {
        destroy();
    }

    void loadFromPath(SDL_Renderer* renderer, const std::string& fontpath, ColorRGBAi color = { 0, 0, 0, 255 }, int ptsize = 30, FontStyle style = FontStyle::NORMAL)
    {
        if (created()) { FC_FreeFont(_font); }
        _font = FC_CreateFont();
        int err = FC_LoadFont(_font, renderer, fontpath.c_str(), ptsize, { color.r,color.g,color.b,color.a }, static_cast<int>(style));
        if (err == 0)
        { std::cerr << "Failed to load font from path: \"" << fontpath << "\" \n"; }
    }

    bool created() { return _font != nullptr; }

    void destroy()
    {
        if (created()) { FC_FreeFont(_font); _font = nullptr; }
    }

    operator FC_Font*() { return _font; }
    operator FC_Font&() { return *_font; }
};