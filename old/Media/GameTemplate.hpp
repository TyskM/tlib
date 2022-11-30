#pragma once

#include "../StateMan.hpp"
#include "../Timer.hpp"
#include "Media.hpp"
#include "ImGui.hpp"
#include "SysQuery.hpp"
#include <format>

/* Usage example:

#define _SILENCE_ALL_CXX20_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Media/GameTemplate.hpp>

GameTemplate game;

struct PlayState : GameState
{
    void input(InputEvent& e) override
    {

    }

    void update(float delta) override
    {

    }

    void draw(float delta) override
    {

    }
};

int main()
{
    game.create();
    PlayState playState;
    game.start(playState);
}

*/



struct GameState : State
{
    virtual void input(InputEvent& e) { }
    virtual void update(float delta) { }
    virtual void draw(float delta) { }
};
using GameStateMan = StateMan<GameState>;

struct GameTemplate
{
    Window window;
    Renderer renderer;
    ImGuiWrapper imgui;
    FPSLimit fpslimit{60};
    GameStateMan stateMan;
    Timer deltaTimer;
    ColorRGBAi clearColor = { 0, 0, 0 };
    bool running;
    float delta; // Updated after update() is called

    void create(const char* winName = "Window", int winx = 1280, int winy = 720, bool useLinearFiltering = false, int renderScaleQuality = 2)
    {
        window.create(winName, winx, winy);
        renderer.create(window, useLinearFiltering, renderScaleQuality);
        imgui.create(window, renderer);
    }

    void start(GameState& gameState)
    {
        if (renderer._renderer == nullptr) { create(); }

        stateMan.pushState(&gameState);

        running = true;
        while (running)
        {
            Input::update();
            InputEvent e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) { running = false; }
                imgui.processEvent(&e);
                stateMan.getState()->input(e);
            }

            renderer.clear(clearColor);
            imgui.newFrame();

            delta = deltaTimer.restart().asSeconds();
            stateMan.getState()->update(delta);
            stateMan.getState()->draw(delta);

            imgui.render();
            renderer.present();
            fpslimit.wait();
        }
    }
};
