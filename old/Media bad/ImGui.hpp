#pragma once

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui_impl_sdl.h>
#include <thirdparty/imgui_impl_sdl.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui-SFML.h>
#include <SFML/OpenGL.hpp>

namespace MyGui
{
    sf::Clock deltaClock;

    SDL_Cursor* arrowCursor;
    SDL_Cursor* textCursor;
    SDL_Cursor* sizeAllCursor;
    SDL_Cursor* sizeNSCursor;
    SDL_Cursor* sizeEWCursor;
    SDL_Cursor* sizeNESWCursor;
    SDL_Cursor* sizeNWSECursor;
    SDL_Cursor* handCursor;
    SDL_Cursor* notAllowedCursor;
    ImGuiMouseCursor lastCursor = ImGuiMouseCursor_COUNT;

    bool _processEvent(const SDL_Event* event)
    {
        ImGuiIO& io = ImGui::GetIO();

        switch (event->type)
        {
        case SDL_MOUSEMOTION:
        {
            io.AddMousePosEvent((float)event->motion.x, (float)event->motion.y);
            return true;
        }
        case SDL_MOUSEWHEEL:
        {
            float wheel_x = (event->wheel.x > 0) ? 1.0f : (event->wheel.x < 0) ? -1.0f : 0.0f;
            float wheel_y = (event->wheel.y > 0) ? 1.0f : (event->wheel.y < 0) ? -1.0f : 0.0f;
            io.AddMouseWheelEvent(wheel_x, wheel_y);
            return true;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            int mouse_button = -1;
            if (event->button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
            if (event->button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
            if (event->button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
            if (event->button.button == SDL_BUTTON_X1) { mouse_button = 3; }
            if (event->button.button == SDL_BUTTON_X2) { mouse_button = 4; }
            if (mouse_button == -1)
                break;
            io.AddMouseButtonEvent(mouse_button, (event->type == SDL_MOUSEBUTTONDOWN));
            //bd->MouseButtonsDown = (event->type == SDL_MOUSEBUTTONDOWN) ? (bd->MouseButtonsDown | (1 << mouse_button)) : (bd->MouseButtonsDown & ~(1 << mouse_button));
            return true;
        }
        case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            ImGui_ImplSDL2_UpdateKeyModifiers((SDL_Keymod)event->key.keysym.mod);
            ImGuiKey key = ImGui_ImplSDL2_KeycodeToImGuiKey(event->key.keysym.sym);
            io.AddKeyEvent(key, (event->type == SDL_KEYDOWN));
            io.SetKeyEventNativeData(key, event->key.keysym.sym, event->key.keysym.scancode, event->key.keysym.scancode); // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
            return true;
        }
        case SDL_WINDOWEVENT:
        {
            // - When capturing mouse, SDL will send a bunch of conflicting LEAVE/ENTER event on every mouse move, but the final ENTER tends to be right.
            // - However we won't get a correct LEAVE event for a captured window.
            // - In some cases, when detaching a window from main viewport SDL may send SDL_WINDOWEVENT_ENTER one frame too late,
            //   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag operation by clear mouse position. This is why
            //   we delay process the SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
            Uint8 window_event = event->window.event;
            //if (window_event == SDL_WINDOWEVENT_ENTER)
            //    bd->PendingMouseLeaveFrame = 0;
            //if (window_event == SDL_WINDOWEVENT_LEAVE)
            //    bd->PendingMouseLeaveFrame = ImGui::GetFrameCount() + 1;
            if (window_event == SDL_WINDOWEVENT_FOCUS_GAINED)
                io.AddFocusEvent(true);
            else if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                io.AddFocusEvent(false);
            return true;
        }
        }
        return false;
    }

    void init(Window& window, bool loadDefaultFont = true)
    {
        auto ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);
        ImGui::StyleColorsDark();
        ImGui::SFML::Init(*window.sfmlwindow, loadDefaultFont);

        arrowCursor      = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
        textCursor       = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
        sizeAllCursor    = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
        sizeNSCursor     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
        sizeEWCursor     = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
        sizeNESWCursor   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
        sizeNWSECursor   = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
        handCursor       = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
        notAllowedCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
    }

    void input(SDL_Event& e)
    {
        _processEvent(&e);
    }

    void newFrame(Window& window)
    {
        const auto displaySize = window.sfmlwindow->getSize();

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2(displaySize.x, displaySize.y);
        io.DeltaTime = deltaClock.restart().asSeconds();
        // mouse pos is handled by input func rn

        // Update mouse cursor (SDL handles this)
        const auto cursor = ImGui::GetMouseCursor();
        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0 && cursor != lastCursor)
        {
            lastCursor = cursor;

            if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
            {
                SDL_ShowCursor(false);
            }
            else
            {
                SDL_ShowCursor(true);

                switch (cursor)
                {
                case ImGuiMouseCursor_Arrow:      SDL_SetCursor(arrowCursor      ); break;
                case ImGuiMouseCursor_TextInput:  SDL_SetCursor(textCursor       ); break;
                case ImGuiMouseCursor_ResizeAll:  SDL_SetCursor(sizeAllCursor    ); break;
                case ImGuiMouseCursor_ResizeNS:   SDL_SetCursor(sizeNSCursor     ); break;
                case ImGuiMouseCursor_ResizeEW:   SDL_SetCursor(sizeEWCursor     ); break;
                case ImGuiMouseCursor_ResizeNESW: SDL_SetCursor(sizeNESWCursor   ); break;
                case ImGuiMouseCursor_ResizeNWSE: SDL_SetCursor(sizeNWSECursor   ); break;
                case ImGuiMouseCursor_Hand:       SDL_SetCursor(handCursor       ); break;
                case ImGuiMouseCursor_NotAllowed: SDL_SetCursor(notAllowedCursor ); break;
                default: break;
                }
            }
        }

        ImGui::NewFrame();
    }

    void render(Window& window)
    {
        ImGui::SFML::Render(*window.sfmlwindow);
    }
}