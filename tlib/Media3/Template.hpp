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

struct GameTemplate
{
    Window win;
    Renderer r;
    MyGui imgui;
    Timer dtTimer;
    FPSLimit fps;
    StateMan<GameState> stateMan;

    GameTemplate()
    {
        fps.setFPSLimit(60);
    }

    void create(const char* winTitle = "Window", Vector2i winSize = {1280, 720})
    {
        win.create(winTitle, winSize.x, winSize.y);
        r.create(win);
        imgui.create(win);
    }

    void run(GameState& state)
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

                stateMan.getState()->input(e);
            }
            imgui.newFrame();
            float dt = dtTimer.restart().asSeconds();
            stateMan.getState()->update(dt);
            stateMan.getState()->draw(dt);
            imgui.render();
            win.swap();
            fps.wait();
        }
    }

};
