#pragma once

#include "../StateMan.hpp"
#include "../Timer.hpp"
#include "Media.hpp"
#include "ImGui.hpp"
#include "SysQuery.hpp"
#include <format>

/* Usage example:

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
    bool showDebug;
    bool running;

    void create(const char* winName = "Window", int winx = 1280, int winy = 720, bool useLinearFiltering = false, int renderScaleQuality = 2)
    {
        window.create(winName, winx, winy);
        renderer.create(window, useLinearFiltering, renderScaleQuality);
        imgui.create(window, renderer);
    }

    void start(GameState& gameState, bool showDebug = true)
    {
        if (renderer._renderer == nullptr) { create(); }
        this->showDebug = showDebug;

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

            float delta = deltaTimer.restart().asSeconds();
            stateMan.getState()->update(delta);
            stateMan.getState()->draw(delta);
            if (showDebug)
            {
                const auto meminfo = sysq::getGlobalMemInfo();
                const auto meminfop = sysq::getThisProcessMemUsage();

                ImGui::Begin("Debug##7");
                
                ImGui::Text("Delta: %f", delta);
                ImGui::Text(std::format("CPU Usage: {}%", sysq::getThisProcessCPUUsage()).c_str());
                ImGui::Text(std::format("Phys Ram Used: {} MB", sysq::bytesToMb(meminfop.workingSetSize)).c_str());
                ImGui::Text(std::format("Virt Ram Used: {} MB", sysq::bytesToMb(meminfop.privateUsage)).c_str());
                ImGui::Text(std::format("Phys RAM Avail: {} MB", sysq::bytesToMb(meminfo.availPhysical)).c_str());
                ImGui::Text(std::format("Phys RAM Total: {} MB", sysq::bytesToMb(meminfo.totalPhysical)).c_str());
                ImGui::Text(std::format("Virt RAM Avail: {} MB", sysq::bytesToMb(meminfo.availVirtual)).c_str());
                ImGui::Text(std::format("Virt RAM Total: {} MB", sysq::bytesToMb(meminfo.totalVirtual)).c_str());

                ImGui::End();
            }
            imgui.render();
            renderer.present();
            fpslimit.wait();
        }
    }
};
