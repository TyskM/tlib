#pragma once

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_mouse.h>

#include <SDL2/SDL_joystick.h> // TODO: controller support
#include <SDL2/SDL_gamecontroller.h>

#include "../DataStructures.hpp"
#include <string>
#include <vector>
#include <map>

/* Usage example

Input::Action actPrimary   = { "Primary",    { ActionType::MOUSE, Input::MOUSE_LEFT } };
Input::Action actMoveNorth = { "Move North", { ActionType::KEYBOARD, SDL_SCANCODE_W } };
Input::Action actMoveUp    = { "Move Up",    { ActionType::KEYBOARD, SDL_SCANCODE_W, SDL_Keymod::KMOD_LSHIFT } };

void update()
{
    if (Input::isActionJustPressed(actPrimary))
    { shootGun(); }
    if (Input::isActionJustPressed(actMoveUp))
    { jump(); }
}

*/

// See file for usage example
struct Input
{
    enum class ActionType
    { MOUSE, KEYBOARD };

    struct ActionControl
    {
        ActionControl(ActionType type, int id, SDL_Keymod modifier = KMOD_NONE)
            : type{ type }, id{ id }, modifier{ modifier } { }

        bool strictModChecking = true;
        int modifier = KMOD_NONE; // See SDL_Keymod
        ActionType type;
        int id;
    };

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

    static inline const int MOUSE_LEFT         =  0;
    static inline const int MOUSE_MIDDLE       =  1;
    static inline const int MOUSE_RIGHT        =  2;
    static inline const int MOUSE_X1           =  3;
    static inline const int MOUSE_X2           =  4;
    static inline const int MOUSE_WHEEL_DOWN   =  5;
    static inline const int MOUSE_WHEEL_UP     =  6;
    static inline const int MB_COUNT           =  7;

    static inline std::vector<Uint8> kb;
    static inline std::vector<Uint8> prevkb;

    static inline std::array<Uint8, MB_COUNT> mouse;
    static inline std::array<Uint8, MB_COUNT> prevmouse;

    // Don't update mouse wheel state until update() is called
    static inline bool wheelUpNextUpdate   = false;
    static inline bool wheelDownNextUpdate = false;

    static inline Vector2i mousePos;
    static inline Vector2i prevMousePos;
    static inline Vector2i mouseDelta;

    static inline void input(const SDL_Event& e)
    {
        if      (e.type == SDL_MOUSEWHEEL && e.wheel.y > 0) { wheelUpNextUpdate   = true; }
        else if (e.type == SDL_MOUSEWHEEL && e.wheel.y < 0) { wheelDownNextUpdate = true; }
    }

    // Call before update loop
    static inline void update()
    {
        prevkb = kb;

        int keyCount;
        auto rawkb = SDL_GetKeyboardState(&keyCount);
        kb = std::vector<Uint8>(rawkb, rawkb + keyCount);

        prevmouse = mouse;
        prevMousePos = mousePos;
        auto tempMouse = SDL_GetMouseState(&mousePos.x, &mousePos.y);
        for (int i = 0; i <= MOUSE_X2; i++)
        {
            mouse[i] = (tempMouse & (1 << i) /* frick sdl button SDL_BUTTON(i)*/ );
        }

        mouseDelta = mousePos - prevMousePos;
        mouse[MOUSE_WHEEL_UP]   = wheelUpNextUpdate;
        mouse[MOUSE_WHEEL_DOWN] = wheelDownNextUpdate;
        wheelUpNextUpdate   = false;
        wheelDownNextUpdate = false;
    }

    static inline bool isActionPressed(const Action& action)
    {
        if (!_verifyAction(action)) { return false; }
        for (auto& ctrl : action.controls)
        {
            if (!_verifyControl(ctrl)) return false;
            switch (ctrl.type)
            {
            case ActionType::MOUSE:    if (isMousePressed(ctrl.id)) return true; break;
            case ActionType::KEYBOARD: if (isKeyPressed(ctrl.id))   return true; break;
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
            if (!_verifyControl(ctrl)) return false;
            switch (ctrl.type)
            {
            case ActionType::MOUSE:    if (isMouseJustPressed(ctrl.id)) return true; break;
            case ActionType::KEYBOARD: if (isKeyJustPressed(ctrl.id))   return true; break;
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
            if (!_verifyControl(ctrl)) return false;
            switch (ctrl.type)
            {
            case ActionType::MOUSE:    return isMouseJustReleased(ctrl.id); break;
            case ActionType::KEYBOARD: return isKeyJustReleased(ctrl.id);   break;
            default: return false; break;
            }
        }
        return false;
    }

    static inline bool isKeyPressed     (int key) { return kb[key]; }
    static inline bool isKeyJustPressed (int key) { return kb[key] && !prevkb[key]; }
    static inline bool isKeyReleased    (int key) { return !isKeyPressed(key); }
    static inline bool isKeyJustReleased(int key) { return !kb[key] && prevkb[key]; }

    static inline bool isMousePressed     (int button) { return (mouse[button]); }
    static inline bool isMouseJustPressed (int button)
    {
        if (button >= MOUSE_WHEEL_DOWN) { return isMousePressed(button); }
        else { return (mouse[button] && !prevmouse[button]); }
    }

    static inline bool isMouseReleased    (int button) { return !isMousePressed(button); }
    static inline bool isMouseJustReleased(int button)
    {
        if (button >= MOUSE_WHEEL_DOWN) { return isMouseReleased(button); }
        else { return !mouse[button] && prevmouse[button]; }
    }

    // see SDL_Keymod
    // Returns a pretty string showing the SDL_Keymod combination
    // modToString(SDL_Keymod::KMOD_LCTRL | SDL_Keymod::KMOD_LSHIFT) -> "LShift + LCtrl"
    static std::string modToString(int mod)
    {
        std::string str;
        static const std::map<SDL_Keymod, std::string> modMap =
        {
            { KMOD_NONE  , "None"   },
            { KMOD_LSHIFT, "LShift" },
            { KMOD_RSHIFT, "RShift" },
            { KMOD_LCTRL , "LCtrl"  },
            { KMOD_RCTRL , "RCtrl"  },
            { KMOD_LALT  , "LAlt"   },
            { KMOD_RALT  , "RAlt"   },
            { KMOD_LGUI  , "LGui"   },
            { KMOD_RGUI  , "RGui"   },
            { KMOD_NUM   , "Num"    },
            { KMOD_CAPS  , "Caps"   },
            { KMOD_MODE  , "Mode"   },
            { KMOD_SCROLL, "Scroll" }
        };

        size_t modCount = 0;
        for (auto& [k, v] : modMap)
        {
            if (k & mod)
            {
                if (modCount > 0)
                { str += " + "; }
                str += v;
                ++modCount;
            }
        }
        
        return str;
    }

    static std::string controlToString(const ActionControl& ctrl)
    {
        std::string str;
        if (ctrl.modifier != SDL_Keymod::KMOD_NONE)
        {
            str = modToString(ctrl.modifier) + " + ";
        }
        switch (ctrl.type)
        {
            case ActionType::MOUSE:
                switch (ctrl.id)
                {
                    case MOUSE_LEFT      : str += "Mouse Left";       break;
                    case MOUSE_MIDDLE    : str += "Mouse Middle";     break;
                    case MOUSE_RIGHT     : str += "Mouse Right";      break;
                    case MOUSE_X1        : str += "Mouse X1";         break;
                    case MOUSE_X2        : str += "Mouse X2";         break;
                    case MOUSE_WHEEL_DOWN: str += "Mouse Wheel Down"; break;
                    case MOUSE_WHEEL_UP  : str += "Mouse Wheel Up";   break;
                    default:               str += "Unknown";          break;
                }
                break;
            case ActionType::KEYBOARD:
                str += std::string(SDL_GetScancodeName((SDL_Scancode)ctrl.id));
                break;
            default: str += "Unknown"; break;
        }

        return str;
    }

    static inline bool _verifyAction(const Action& act)
    {
        return act.controls.size() > 0;
    }

    static inline bool _verifyControl(const ActionControl& ctrl)
    {
        if (ctrl.strictModChecking)
            return ctrl.modifier == SDL_GetModState();
        else
            return ctrl.modifier == KMOD_NONE || ctrl.modifier == SDL_GetModState();
    }
}; 
using ActionType = Input::ActionType;
using Action = Input::Action;