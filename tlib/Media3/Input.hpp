#pragma once

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

#include <SDL2/SDL_joystick.h> // TODO: controller support
#include <SDL2/SDL_gamecontroller.h>

#include "../DataStructures.hpp"
#include <string>
#include <vector>

struct Input
{
    enum ActionType
    {
        MOUSE, KEYBOARD
    };
    using AT = ActionType;

    struct ActionControl
    {
        ActionControl(ActionType type, int id) : type{ type }, id{ id } { }

        ActionType type;
        int id;
    };
    using AC = ActionControl;

    struct Action
    {
        Action(const std::string& name, const Input::ActionControl& singleAction)
        {
            this->name = name;
            controls.push_back(singleAction);
        }

        Action(const std::string& name, const std::initializer_list<Input::ActionControl>& actions)
        {
            this->name = name;
            controls.reserve(actions.size());
            for (auto& act : actions)
            { controls.push_back(act); }
        }

        Action(const std::string& name, const Input::ActionType& actType, const int id)
        {
            this->name = name;
            controls.emplace_back(actType, id);
        }

        std::string name;
        std::vector<Input::ActionControl> controls;
    };
    using Act = Action;

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
        if (!_verifyAction(action)) { return false; }
        for (auto& ctrl : action.controls)
        {
            switch (ctrl.type)
            {
            case MOUSE:    if (isMousePressed(ctrl.id)) return true; break;
            case KEYBOARD: if (isKeyPressed(ctrl.id))   return true; break;
            default: return false; break;
            }
        }
        return false;
    }

    static inline bool isActionJustPressed(const Action& action)
    {
        if (!_verifyAction(action)) { return false; }
        for (auto& ctrl : action.controls)
        {
            switch (ctrl.type)
            {
            case MOUSE:    if (isMouseJustPressed(ctrl.id)) return true; break;
            case KEYBOARD: if (isKeyJustPressed(ctrl.id))   return true; break;
            default: return false; break;
            }
        }
        return false;
    }

    static inline bool isActionReleased(const Action& action)
    {
        if (!_verifyAction(action)) { return false; }
        return !isActionPressed(action);
    }

    static inline bool isActionJustReleased(const Action& action)
    {
        if (!_verifyAction(action)) { return false; }
        for (auto& ctrl : action.controls)
        {
            switch (ctrl.type)
            {
            case MOUSE:    if (isMouseJustReleased(ctrl.id)) return true; break;
            case KEYBOARD: if (isKeyJustReleased(ctrl.id))   return true; break;
            default: return false; break;
            }
        }
        return false;
    }

    static inline bool isKeyPressed     (int key) { return kb[key]; }
    static inline bool isKeyJustPressed (int key) { return kb[key] && !prevkb[key]; }
    static inline bool isKeyReleased    (int key) { return !isKeyPressed(key); }
    static inline bool isKeyJustReleased(int key) { return !kb[key] && prevkb[key]; }

    static inline bool isMousePressed     (int button) { return (mouse & SDL_BUTTON(button)); }
    static inline bool isMouseJustPressed (int button) { return (mouse & SDL_BUTTON(button) && !(prevmouse & SDL_BUTTON(button))); }
    static inline bool isMouseReleased    (int button) { return !isMousePressed(button); }
    static inline bool isMouseJustReleased(int button) { return !(mouse & SDL_BUTTON(button) && (prevmouse & SDL_BUTTON(button))); }

    static inline bool _verifyAction(const Action& act)
    {
        return act.controls.size() > 0;
    }
}; 
using ActionType = Input::ActionType;
using Action = Input::Action;