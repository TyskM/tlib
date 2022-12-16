
// Usage example:

//#include <MediaGL/GameTemplate.hpp>
//
//GameTemplate game;
//Renderer& renderer         = game.renderer;
//Window& window             = game.win;
//StateMan<GameState>& state = game.stateMan;
//
//struct PlayState : GameState
//{
//    void input(const SDL_Event& e) override
//    {
//
//    }
//
//    void update(float delta) override
//    {
//
//    }
//
//    void draw(float delta) override
//    {
//
//    }
//};
//
//int main()
//{
//    game.create();
//    PlayState playState;
//    game.start(playState);
//    return 0;
//}

#pragma once

#include "../StateMan.hpp"
#include "../Timer.hpp"
#include "Window.hpp"
#include "Texture.hpp"
#include "Renderer.hpp"
#include "ImGui.hpp"
#include "Input.hpp"
#include "Helpers.hpp"
#include "FPSLimit.hpp"

struct GameState : State
{
    virtual void input(const SDL_Event& e) { }
    virtual void update(float delta) { }
    virtual void draw(float delta)   { }
};

struct WindowTemplate
{
    Window win;
    Timer dtTimer;
    FPSLimit fpslimit;
    StateMan<GameState> stateMan;

    WindowTemplate()
    {
        fpslimit.setFPSLimit(60);
    }

    void create(const WindowCreateParams& params = WindowCreateParams())
    {
        win.create(params);
    }

    void start(GameState& state)
    {
        stateMan.pushState(&state);
        dtTimer.restart();

        bool running = true;
        while (running)
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) { running = false; }
                Input::input(e);
                stateMan.getState()->input(e);
            }
            Input::update();
            float dt = dtTimer.restart().asSeconds();
            stateMan.getState()->update(dt);
            stateMan.getState()->draw(dt);
            win.swap();
            fpslimit.wait();
        }
    }

};

struct GameTemplate
{
    Window              win;
    Renderer            renderer;
    MyGui               imgui;
    Timer               dtTimer;
    FPSLimit            fpslimit;
    StateMan<GameState> stateMan;

    GameTemplate()
    {
        fpslimit.setFPSLimit(60);
    }

    void create(const WindowCreateParams& params = WindowCreateParams())
    {
        WindowCreateParams p = params;
        p.flags |= WindowFlags::OpenGL;
        win.create(p);
        renderer.create(win);
        imgui.create(renderer);
    }

    void start(GameState& state)
    {
        stateMan.pushState(&state);
        dtTimer.restart();

        bool running = true;
        while (running)
        {
            auto& io = ImGui::GetIO();
            if (!(io.WantCaptureKeyboard || io.WantCaptureMouse)) { Input::update(); }

            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) { running = false; }

                imgui.input(e);
                if (io.WantCaptureKeyboard || io.WantCaptureMouse) { continue; }
                Input::input(e);
                stateMan.getState()->input(e);
            }
            float dt = dtTimer.restart().asSeconds();
            stateMan.getState()->update(dt);
            stateMan.getState()->draw(dt);
            glState.reset();
            win.swap();
            fpslimit.wait();
        }
    }

};
