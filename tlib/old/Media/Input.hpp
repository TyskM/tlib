#pragma once

#include "Window.hpp"
#include "../DataStructures.hpp"
#include <SDL2/SDL.h>
#include <string>

struct Input
{
    enum ActionType
    {
        MOUSE, KEYBOARD
    };

    struct Action
    {
        std::string name;
        ActionType type;
        int id;
    };

    static inline const int MOUSE_LEFT   =  1;
    static inline const int MOUSE_MIDDLE =  2;
    static inline const int MOUSE_RIGHT  =  3;
    static inline const int MOUSE_X1     =  4;
    static inline const int MOUSE_X2     =  5;

    static inline const Uint8* kb;
    static inline const Uint8* prevkb;
    static inline Uint32 mouse;
    static inline Uint32 prevmouse;

    static inline void update()
    {
        prevkb = kb;
        kb = SDL_GetKeyboardState(NULL);
        prevmouse = mouse;
        mouse = SDL_GetMouseState(NULL, NULL);
    }

    static inline bool actionIsPressed(const Action& action)
    {
        switch (action.type)
        {
        case MOUSE: return (mouse & SDL_BUTTON(action.id));
        case KEYBOARD: return kb[action.id];
        default: return false;
        }
    }

    static inline bool actionIsJustPressed(const Action& action)
    {
        switch (action.type)
        {
        case MOUSE: return (mouse & SDL_BUTTON(action.id) && !(prevmouse & SDL_BUTTON(action.id)));
        case KEYBOARD: return kb[action.id] && !prevkb[action.id];
        default: return false;
        }
    }

    static inline bool actionIsReleased(const Action& action)
    { return !actionIsPressed(action); }

    static inline bool actionIsJustReleased(const Action& action)
    {
        switch (action.type)
        {
        case MOUSE: return !(mouse & SDL_BUTTON(action.id) && (prevmouse & SDL_BUTTON(action.id)));
        case KEYBOARD: return !kb[action.id] && prevkb[action.id];
        default: return false;
        }
    }
};
using ActionType = Input::ActionType;
using Action = Input::Action;