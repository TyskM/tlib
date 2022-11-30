#pragma once

#include <SDL2/SDL.h>
#include "../NonAssignable.hpp"
#include "../DataStructures.hpp"
#undef main

constexpr int WINDOWPOS_UNDEFINED = SDL_WINDOWPOS_UNDEFINED;

struct Window : NonAssignable
{
    // Internal vars
    inline static constexpr int _subsys = SDL_INIT_TIMER | SDL_INIT_EVENTS;
    SDL_Window* _window;
    bool _created = false;

    void create(const char* title, int winwidth, int winheight,
                uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE, int winposx = WINDOWPOS_UNDEFINED, int winposy = WINDOWPOS_UNDEFINED, bool vsync = false)
    {
        SDL_Init(_subsys);
        _window = SDL_CreateWindow(title, winposx, winposy, winwidth, winheight, flags);

        if (_window == nullptr)
        { SDL_Log("Could not create a window: %s", SDL_GetError()); abort(); }

        SDL_GL_SetSwapInterval(vsync);

        _created = true;
    }
    
    SDL_Surface* getSurface() const noexcept
    {
        return SDL_GetWindowSurface(_window);
    }

    Vector2i getSize() const noexcept
    {
        Vector2i s;
        SDL_GetWindowSize(_window, &s.x, &s.y);
        return s;
    }

    operator SDL_Window*() { return _window; }
    operator SDL_Window&() { return *_window; }

    ~Window()
    {
        if (_created)
        {
            SDL_DestroyWindow(_window);
            SDL_QuitSubSystem(_subsys);
            _created = false;
        }
    }
};

