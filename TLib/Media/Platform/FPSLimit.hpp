#pragma once

#include <TLib/Media/Platform/SDL2.hpp>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL2_framerate.h>

struct FPSLimit
{
    FPSmanager man;      // Read only
    bool enabled = true; // Read / Write

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

    inline int getFPSLimit() noexcept { return SDL_getFramerate(&man); }

    inline void wait() noexcept
    {
        if (enabled) { SDL_framerateDelay(&man); }
    }
};