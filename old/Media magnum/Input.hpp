#pragma once

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

#include <SDL2/SDL_joystick.h> // TODO: controller support
#include <SDL2/SDL_gamecontroller.h>

#include "../../DataStructures.hpp"
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

    static inline std::vector<Uint8> kb;
    static inline std::vector<Uint8> prevkb;

    static inline Uint32 mouse;
    static inline Uint32 prevmouse;

    static inline void update()
    {
        prevkb = kb;

        int keyCount;
        auto rawkb = SDL_GetKeyboardState(&keyCount);
        kb = std::vector<Uint8>(rawkb, rawkb + keyCount);

        prevmouse = mouse;
        mouse = SDL_GetMouseState(NULL, NULL);
    }

    static inline bool isActionPressed(const Action& action)
    {
        switch (action.type)
        {
        case MOUSE:    return isMousePressed(action.id);
        case KEYBOARD: return isKeyPressed(action.id);
        default: return false;
        }
    }

    static inline bool isActionJustPressed(const Action& action)
    {
        switch (action.type)
        {
        case MOUSE:    return isMouseJustPressed(action.id);
        case KEYBOARD: return isKeyJustPressed(action.id);
        default: return false;
        }
    }

    static inline bool isActionReleased(const Action& action)
    { return !isActionPressed(action); }

    static inline bool isActionJustReleased(const Action& action)
    {
        switch (action.type)
        {
        case MOUSE:    return isMouseJustReleased(action.id);
        case KEYBOARD: return isKeyJustReleased(action.id);
        default: return false;
        }
    }

    static inline bool isKeyPressed     (int key) { return kb[key]; }
    static inline bool isKeyJustPressed (int key) { return kb[key] && !prevkb[key]; }
    static inline bool isKeyReleased    (int key) { return !isKeyPressed(key); }
    static inline bool isKeyJustReleased(int key) { return !kb[key] && prevkb[key]; }

    static inline bool isMousePressed     (int button) { return (mouse & SDL_BUTTON(button)); }
    static inline bool isMouseJustPressed (int button) { return (mouse & SDL_BUTTON(button) && !(prevmouse & SDL_BUTTON(button))); }
    static inline bool isMouseReleased    (int button) { return !isMousePressed(button); }
    static inline bool isMouseJustReleased(int button) { return !(mouse & SDL_BUTTON(button) && (prevmouse & SDL_BUTTON(button))); }
}; 
using ActionType = Input::ActionType;
using Action = Input::Action;