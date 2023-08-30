
#pragma once
#include <TLib/Media/GUI/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Timer.hpp>

namespace agui
{
    class AGUI_BACKEND_DECLSPEC TLibInput : public Input
    {
        Timer clock;

        MouseButtonEnum sdlButtonToAg(uint8_t sdlButton)
        {
            // Extra padding, in case the user has weird mouse
            constexpr int buttonMap[10] = {0, 1, 3, 2, 0, 0, 0, 0, 0, 0};
            return static_cast<MouseButtonEnum>(buttonMap[sdlButton]);
        }

    public:
        TLibInput(void)
        {
            setMouseEnabled(true);
            setKeyboardEnabled(true);
        }

        virtual ~TLibInput(void) { }

        virtual double getTime() const
        { return clock.getElapsedTime().asSeconds(); }

        void processEvent(const SDL_Event& event)
        {
            switch (event.type)
            {
            case SDL_MOUSEMOTION:
            {
                SDL_Keymod mods = SDL_GetModState();
                pushMouseEvent(
                    MouseInput(
                        MouseEvent::MOUSE_MOVE,  // event type
                        MOUSE_BUTTON_NONE,       // mouse button
                        event.motion.x,          // x
                        event.motion.y,          // y
                        0,                       // wheel
                        0,                       // pressure
                        getTime(),               // timestamp
                        mods & KMOD_ALT,         // ALT state
                        mods & KMOD_SHIFT,       // SHIFT state
                        mods & KMOD_CTRL));      // CTRL state
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                SDL_Keymod mods = SDL_GetModState();
                pushMouseEvent(
                    MouseInput(
                        MouseEvent::MOUSE_DOWN,              // event type
                        sdlButtonToAg(event.button.button),  // mouse button
                        event.button.x,                      // x
                        event.button.y,                      // y
                        0,                                   // wheel
                        0,                                   // pressure
                        getTime(),                           // timestamp
                        mods & KMOD_ALT,                     // ALT state
                        mods & KMOD_SHIFT,                   // SHIFT state
                        mods & KMOD_CTRL));                  // CTRL state
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                SDL_Keymod mods = SDL_GetModState();
                pushMouseEvent(
                    MouseInput(
                        MouseEvent::MOUSE_UP,                // event type
                        sdlButtonToAg(event.button.button),  // mouse button
                        event.button.x,                      // x
                        event.button.y,                      // y
                        0,                                   // wheel
                        0,                                   // pressure
                        getTime(),                           // timestamp
                        mods & KMOD_ALT,                     // ALT state
                        mods & KMOD_SHIFT,                   // SHIFT state
                        mods & KMOD_CTRL));                  // CTRL state
                break;
            }
            case SDL_MOUSEWHEEL:
            {
                SDL_Keymod mods = SDL_GetModState();

                agui::MouseEvent::MouseEventEnum e = MouseEvent::MOUSE_WHEEL_DOWN;
                if (event.wheel.y > 0) { e = MouseEvent::MOUSE_WHEEL_UP; }

                pushMouseEvent(
                    MouseInput(
                        e,                  // event type
                        MOUSE_BUTTON_NONE,  // mouse button
                        event.wheel.mouseX, // x
                        event.wheel.mouseY, // y
                        event.wheel.y,      // wheel
                        0,                  // pressure
                        getTime(),          // timestamp
                        mods & KMOD_ALT,    // ALT state
                        mods & KMOD_SHIFT,  // SHIFT state
                        mods & KMOD_CTRL)); // CTRL state
                break;
            }
            default: break;
            }
        }
    };
}