#pragma once

#include "../StateMan.hpp"
#include "../Timer.hpp"
#include "Media.hpp"
#include "ImGui.hpp"
#include "SysQuery.hpp"
#include <format>

struct GameState : State
{
    virtual void updatedraw(float delta) {}
    virtual void input(Event& e) {}
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
    bool showDebug;
    bool running;

    void create(const char* winName = "Window", int winx = 1280, int winy = 720)
    {
        window.create(winName, winx, winy);
        renderer.create(window);
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
            Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) { running = false; }
                imgui.processEvent(&e);
                stateMan.getState()->input(e);
            }

            imgui.newFrame();
            renderer.clear();

            float delta = deltaTimer.restart().asSeconds();
            stateMan.getState()->updatedraw(delta);

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
