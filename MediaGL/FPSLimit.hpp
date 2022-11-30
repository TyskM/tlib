#pragma once

#include <SDL2/SDL_timer.h>
#include <SDL2/SDL2_framerate.h>

struct FPSLimit
{
    FPSmanager man;
    bool enabled = true;

    FPSLimit(uint32_t limit = 60)
    {
        SDL_initFramerate(&man);
        setFPSLimit(limit);
    }

    inline void setFPSLimit(uint32_t limit)
    {
        ASSERT(limit > 0.f);
        SDL_setFramerate(&man, limit);
    }

    inline void setEnabled(bool v) { enabled = v; }

    inline double getFPSLimit() noexcept { return SDL_getFramerate(&man); }

    inline void wait() noexcept
    {
        if (enabled) { SDL_framerateDelay(&man); }
    }
};