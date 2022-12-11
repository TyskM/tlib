#pragma once


#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Event.hpp>
#include "../DataStructures.hpp"
#include <string>
#include <vector>


/*
This classes key / mouse / action checking methods (ie. isMouseJustPressed(), isActionReleased())
are meant to be called in your update function.
Calling them in your input() function probably won't work right.

Be sure to call input(Event&) in your event loop,
and call update() !AT THE END! of your update loop.

TODO: readd mouse wheel support
TODO: add examples
*/
struct Input
{
    enum ActionType
    {
        MOUSE, KEYBOARD
    };

    struct ActionControl
    {
        ActionControl(ActionType type, int id) : type{ type }, id{ id } { }

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

    static inline std::array<uint8_t, sf::Keyboard::KeyCount> kb;
    static inline std::array<uint8_t, sf::Keyboard::KeyCount> prevkb;

    static inline std::array<uint8_t, sf::Mouse::ButtonCount> mouse;
    static inline std::array<uint8_t, sf::Mouse::ButtonCount> prevmouse;

    static inline void input(const sf::Event& e)
    {
        switch (e.type)
        {
            case sf::Event::KeyPressed:
                kb[e.key.code] = 1;
                break;
            case sf::Event::KeyReleased:
                kb[e.key.code] = 0;
                break;
            case sf::Event::MouseButtonPressed:
                mouse[e.mouseButton.button] = 1;
                break;
            case sf::Event::MouseButtonReleased:
                mouse[e.mouseButton.button] = 0;
                break;
            default: break;
        }
    }

    // THIS NEEDS TO BE CALLED AT THE END OF THE GAME LOOP
    // NOT BEFORE, OR THE IN THE MIDDLE, YOU CALL IT AT THE END.
    // THE END
    static inline void update()
    {
        prevkb    = kb;
        prevmouse = mouse;
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
            case MOUSE:    return isMouseJustReleased(ctrl.id); break;
            case KEYBOARD: return isKeyJustReleased(ctrl.id);   break;
            default: return false; break;
            }
        }
        return false;
    }

    static inline bool isKeyPressed     (const int key) { return kb[key]; }
    static inline bool isKeyJustPressed (const int key) { return kb[key] && !prevkb[key]; }
    static inline bool isKeyReleased    (const int key) { return !isKeyPressed(key); }
    static inline bool isKeyJustReleased(const int key) { return !kb[key] && prevkb[key]; }

    static inline bool isMousePressed     (const int button) { return (mouse[button]); }
    static inline bool isMouseJustPressed (const int button) { return (mouse[button] && !prevmouse[button]); }
    static inline bool isMouseReleased    (const int button) { return !isMousePressed(button); }
    static inline bool isMouseJustReleased(const int button) { return !mouse[button] && prevmouse[button]; }

    static inline bool _verifyAction(const Action& act)
    {
        return act.controls.size() > 0;
    }
}; 
using ActionType = Input::ActionType;
using Action = Input::Action;