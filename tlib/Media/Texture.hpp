#pragma once

#include <SDL2/SDL.h>
#include <string>
#include "../NonAssignable.hpp"
#include "Renderer.hpp"

struct Texture : NonCopyable
{
    SDL_Texture* _texture = nullptr;

    Texture() = default;
    Texture(Texture&& t) noexcept
    {
        if (_texture != nullptr) { SDL_DestroyTexture(_texture); }
        _texture = t._texture;
        t._texture = nullptr;
    }

    Texture& operator=(Texture&& t) noexcept
    {
        if (_texture != nullptr) { SDL_DestroyTexture(_texture); }
        _texture = t._texture;
        t._texture = nullptr;
        return *this;
    }

    Texture(SDL_Texture* tex) : _texture{ tex } { }

    Texture(Renderer& renderer, const std::string& path)
    { loadFromPath(renderer, path); }

    ~Texture() { if (_texture != nullptr) { SDL_DestroyTexture(_texture); } }

    inline void loadFromPath(Renderer& renderer, const std::string& path) noexcept
    {
        if (_texture != nullptr) { SDL_DestroyTexture(_texture); }
        _texture = renderer.loadTexture(path);
    }

    Vector2i getSize() const
    {
        Vector2i rsize;
        SDL_QueryTexture(_texture, NULL, NULL, &rsize.x, &rsize.y);
        return rsize;
    }

    uint32_t getFormat() const
    {
        uint32_t rformat;
        SDL_QueryTexture(_texture, &rformat, NULL, NULL, NULL);
        return rformat;
    }

    operator SDL_Texture*() { return _texture; }
};