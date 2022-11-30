#pragma once

#include <functional>
#include <SDL2/SDL.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include "../Macros.hpp"
#include "../DataStructures.hpp"
#include "../Timer.hpp"
#include "../StateMan.hpp"
#include "../StateMan.hpp"
#include "../NonAssignable.hpp"
#include "Input.hpp"
#include "View.hpp"

struct GameState : State
{
    virtual void input(const SDL_Event& e) { }
    virtual void update(float delta) { }
    virtual void draw(float delta) { }
};
using GameStateMan = StateMan<GameState>;

struct Window : NonAssignable
{
    // Internal vars
    inline static constexpr int _subsys = SDL_INIT_TIMER | SDL_INIT_EVENTS;
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;

    Timer deltaTimer;
    std::vector<std::function<void(const SDL_Event&)>> eventSink;
    GameStateMan stateMan;

    void create(const char* title, int winwidth, int winheight,
        uint32_t flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE, int winposx = SDL_WINDOWPOS_UNDEFINED, int winposy = SDL_WINDOWPOS_UNDEFINED, bool vsync = false)
    {
        SDL_Init(_subsys);
        _window = SDL_CreateWindow(title, winposx, winposy, winwidth, winheight, flags);

        if (_window == nullptr)
        { SDL_Log("Could not create a window: %s", SDL_GetError()); abort(); }

        // IMGUI SETUP
        const char* glsl_version = "#version 130";
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        SDL_GL_MakeCurrent(_window, _glContext);
        SDL_GL_SetSwapInterval(1); // Enable vsync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(_window, _glContext);
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    operator SDL_Window*() { return _window; }
    operator SDL_Window&() { return *_window; }

    bool created() { return _window != nullptr; }

    ~Window()
    {
        if (created())
        {
            SDL_GL_DeleteContext(_glContext);
            SDL_DestroyWindow(_window);
            SDL_QuitSubSystem(_subsys);
        }
    }

    View getDefaultView()
    {
        View v;
        const auto fb = Vector2f( framebufferSize().x(), framebufferSize().y() );
        v.center.x = fb.x / 2.f;
        v.center.y = fb.y / 2.f;
        v.size = fb;
        return v;
    }

    inline void setTargetFramerate(uint32_t fps) { }
};