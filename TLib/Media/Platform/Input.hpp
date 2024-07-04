#pragma once

// TODO: controller support

#include <TLib/Types/Types.hpp>
#include <TLib/Media/Platform/SDL2.hpp>
#include <TLib/String.hpp>
#include <TLib/Containers/Vector.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Containers/Array.hpp>

/* Usage example

Input::Action actPrimary   = { "Primary",    { Input::ActionType::MOUSE, Input::MOUSE_LEFT } };
Input::Action actMoveNorth = { "Move North", { Input::ActionType::KEYBOARD, SDL_SCANCODE_W } };
Input::Action actMoveUp    = { "Move Up",    { Input::ActionType::KEYBOARD, SDL_SCANCODE_W, SDL_Keymod::KMOD_LSHIFT } };

void update()
{
    if (Input::isActionJustPressed(actPrimary))
    { shootGun(); }
    if (Input::isActionJustPressed(actMoveUp))
    { jump(); }
}

*/
struct Input
{
public:
    enum class ActionType
    { MOUSE, KEYBOARD };

    struct ActionControl
    {
        ActionControl() { }
        ActionControl(ActionType type, int id, SDL_Keymod modifier = KMOD_NONE)
            : type{ type }, id{ id }, modifier{ modifier } { }

        bool valid() const { return id != -1; }
        
        void clear()
        {
            *this = ActionControl();
        }

        int        modifier = KMOD_NONE; // See SDL_Keymod
        ActionType type     = ActionType::KEYBOARD;
        int        id       = -1;
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

        String name;
        Vector<Input::ActionControl> controls;
    };
    using Act = Action;

    static inline Vector2i mousePos;      // Read-only
    static inline Vector2i prevMousePos; // Read-only
    static inline Vector2i mouseDelta;  // Read-only

    static inline const int MOUSE_LEFT         =  0;
    static inline const int MOUSE_MIDDLE       =  1;
    static inline const int MOUSE_RIGHT        =  2;
    static inline const int MOUSE_X1           =  3;
    static inline const int MOUSE_X2           =  4;
    static inline const int MOUSE_WHEEL_DOWN   =  5;
    static inline const int MOUSE_WHEEL_UP     =  6;
    static inline const int MB_COUNT           =  7;

private:
    static inline Vector<Uint8> kb;
    static inline Vector<Uint8> prevkb;

    static inline Array<Uint8, MB_COUNT> mouse;
    static inline Array<Uint8, MB_COUNT> prevmouse;

    // Don't update mouse wheel state until update() is called
    static inline bool wheelUpNextUpdate   = false;
    static inline bool wheelDownNextUpdate = false;

    static inline ActionControl lastInput;

public:
    static inline void init()
    {
        // Fill the buffers
        updateKeyboard(); updateKeyboard();
        updateMouse();    updateMouse();
    }

    // Call in your input loop
    static inline void input(const SDL_Event& e)
    {
        if      (e.type == SDL_MOUSEWHEEL && e.wheel.y > 0) { wheelUpNextUpdate   = true; }
        else if (e.type == SDL_MOUSEWHEEL && e.wheel.y < 0) { wheelDownNextUpdate = true; }

        switch (e.type)
        {
        case SDL_KEYUP:         lastInput = ActionControl(ActionType::KEYBOARD, e.key.keysym.scancode, static_cast<SDL_Keymod>(e.key.keysym.mod)); break;
        case SDL_MOUSEBUTTONUP: lastInput = ActionControl(ActionType::MOUSE,    e.button.button-1); break;

        case SDL_MOUSEWHEEL:
            if (e.wheel.y > 0)
            { lastInput = ActionControl(ActionType::MOUSE, MOUSE_WHEEL_UP); }
            else if (e.wheel.y < 0)
            { lastInput = ActionControl(ActionType::MOUSE, MOUSE_WHEEL_DOWN); }
            break;

        default: break;
        }
    }

    // Call before update loop
    // This calls updateKeyboard(), and updateMouse()
    static inline void update()
    {
        updateKeyboard();
        updateMouse();
    }

    static inline void updateKeyboard()
    {
        prevkb = kb;

        int keyCount;
        auto rawkb = SDL_GetKeyboardState(&keyCount);
        kb = Vector<Uint8>(rawkb, rawkb + keyCount);
    }

    static inline void updateMouse()
    {
        prevmouse = mouse;
        prevMousePos = mousePos;
        auto tempMouse = SDL_GetMouseState(&mousePos.x, &mousePos.y);

        for (int i = 0; i <= MOUSE_X2; i++)
        {
            mouse[i] = (tempMouse & (1 << i) /* frick sdl button SDL_BUTTON(i)*/ );
        }

        int x, y;
        SDL_GetRelativeMouseState(&x, &y);
        mouseDelta = { x, y };
        mouse[MOUSE_WHEEL_UP]   = wheelUpNextUpdate;
        mouse[MOUSE_WHEEL_DOWN] = wheelDownNextUpdate;
        wheelUpNextUpdate   = false;
        wheelDownNextUpdate = false;
    }

    static inline ActionControl getLastInput()
    { return lastInput; }

    static inline void clearLastInput()
    { lastInput = ActionControl(); }

    //// Action input

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

    //// Raw keyboard input

    // See SDL_SCANCODE_
    static inline bool isKeyPressed     (int key) { return kb[key]; }

    // See SDL_SCANCODE_
    static inline bool isKeyJustPressed (int key) { return kb[key] && !prevkb[key]; }

    // See SDL_SCANCODE_
    static inline bool isKeyReleased    (int key) { return !isKeyPressed(key); }

    // See SDL_SCANCODE_
    static inline bool isKeyJustReleased(int key) { return !kb[key] && prevkb[key]; }

    //// Raw mouse input

    // See Input::MOUSE_
    static inline bool isMousePressed     (int button) { return (mouse[button]); }

    // See Input::MOUSE_
    static inline bool isMouseJustPressed (int button)
    {
        if (button >= MOUSE_WHEEL_DOWN) { return isMousePressed(button); }
        else { return (mouse[button] && !prevmouse[button]); }
    }

    // See Input::MOUSE_
    static inline bool isMouseReleased    (int button) { return !isMousePressed(button); }

    // See Input::MOUSE_
    static inline bool isMouseJustReleased(int button)
    {
        if (button >= MOUSE_WHEEL_DOWN) { return isMouseReleased(button); }
        else { return !mouse[button] && prevmouse[button]; }
    }

    //// String junk

    static inline const UnorderedMap<int, String> mouseButtonNameMap =
    {
        { MOUSE_LEFT      , "Mouse Left"       },
        { MOUSE_MIDDLE    , "Mouse Middle"     },
        { MOUSE_RIGHT     , "Mouse Right"      },
        { MOUSE_X1        , "Mouse X1"         },
        { MOUSE_X2        , "Mouse X2"         },
        { MOUSE_WHEEL_DOWN, "Mouse Wheel Down" },
        { MOUSE_WHEEL_UP  , "Mouse Wheel Up"   }
    };

    static inline const UnorderedMap<int, String> mouseButtonNameMapShort =
    {
        { MOUSE_LEFT      , "LMB"         },
        { MOUSE_MIDDLE    , "MMB"         },
        { MOUSE_RIGHT     , "RMB"         },
        { MOUSE_X1        , "MX1"         },
        { MOUSE_X2        , "MX2"         },
        { MOUSE_WHEEL_DOWN, "Wheel Down"  },
        { MOUSE_WHEEL_UP  , "Wheel Up"    }
    };

    static inline const UnorderedMap<SDL_Keymod, String> modMap =
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

    // see SDL_Keymod
    // Returns a pretty string showing the SDL_Keymod combination
    // modToString(SDL_Keymod::KMOD_LCTRL | SDL_Keymod::KMOD_LSHIFT) -> "LShift + LCtrl"
    static std::string modToString(int mod)
    {
        std::string str;

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

    static String controlToStringShort(const ActionControl& ctrl)
    {
        String str;
        if (ctrl.modifier != SDL_Keymod::KMOD_NONE)
        {
            str = modToString(ctrl.modifier) + "+";
        }
        switch (ctrl.type)
        {
            case ActionType::MOUSE:
                if (mouseButtonNameMapShort.contains(ctrl.id))
                { str += mouseButtonNameMapShort.at(ctrl.id); }
                else { str += "?"; }
                break;
            case ActionType::KEYBOARD:
            {
                String name = SDL_GetScancodeName((SDL_Scancode)ctrl.id);
                if (name == "") { str += "?"; }
                else { str += name; }
                break;
            }
            default: str += "?"; break;
        }

        return str;
    }

    static String controlToString(const ActionControl& ctrl)
    {
        String str;
        if (ctrl.modifier != SDL_Keymod::KMOD_NONE)
        {
            str = modToString(ctrl.modifier) + " + ";
        }
        switch (ctrl.type)
        {
            case ActionType::MOUSE:
                if (mouseButtonNameMap.contains(ctrl.id))
                { str += mouseButtonNameMap.contains(ctrl.id); }
                else { str += "?"; }
                break;
            case ActionType::KEYBOARD:
            {
                String name = SDL_GetScancodeName((SDL_Scancode)ctrl.id);
                if (name == "") { str += "?"; }
                else { str += name; }
                break;
            }
            default: str += "?"; break;
        }

        return str;
    }

private:
    static inline bool _verifyAction(const Action& act)
    {
        return act.controls.size() > 0;
    }

    static inline bool _verifyControl(const ActionControl& ctrl)
    {
        const auto modkeys = KMOD_CTRL | KMOD_SHIFT | KMOD_ALT | KMOD_GUI;
        const auto modState = SDL_GetModState();
        const bool MOD_NONE = !(modState & modkeys);
        return ctrl.modifier == (modState & modkeys);
    }
}; 
