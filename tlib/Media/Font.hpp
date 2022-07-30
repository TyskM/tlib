#pragma once

#include <SDL2/SDL_ttf.h>
#include <string>
#include "../Macros.hpp"
#include "../thirdparty/SDL_FontCache.h"
#include "../NonAssignable.hpp"

struct Text
{
    SDL_Texture* texture;
    Vector2i dimensions;

    operator SDL_Texture*() { return texture; }
};

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
    bool _created = false;

    Font() = default;
    Font(SDL_Renderer* renderer, const std::string& fontpath, ColorRGBAi color = { 0, 0, 0, 255 }, int ptsize = 30, FontStyle style = FontStyle::NORMAL)
    {
        loadFromPath(renderer, fontpath, color, ptsize);
    }

    ~Font()
    {
        if (_created) { FC_FreeFont(_font); }
    }

    void loadFromPath(SDL_Renderer* renderer, const std::string& fontpath, ColorRGBAi color = { 0, 0, 0, 255 }, int ptsize = 30, FontStyle style = FontStyle::NORMAL)
    {
        if (_created) { FC_FreeFont(_font); }
        _font = FC_CreateFont();
        int err = FC_LoadFont(_font, renderer, fontpath.c_str(), ptsize, { color.r,color.g,color.b,color.a }, static_cast<int>(style));
        if (err == 0)
        { std::cerr << "Failed to load font from path: \"" << fontpath << "\" \n"; }
        _created = true;
    }

    operator FC_Font*() { return _font; }
    operator FC_Font&() { return *_font; }
};