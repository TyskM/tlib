#pragma once

#include "Window.hpp"

struct GameTemplate
{
    void start()
    {
        bool running = true;
        while (running)
        {
            // Input
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                const auto& io = ImGui::GetIO();
                if (io.WantCaptureKeyboard || io.WantCaptureMouse) { return; }

                Input::update();
                for (auto& func : eventSink) { func(e); }
                stateMan.getState()->input(e);
            }

            // Update / draw
            float delta = deltaTimer.restart().asSeconds();
            imgui.newFrame();

            stateMan.getState()->update(delta);
            stateMan.getState()->draw(delta);
            imgui.updateApplicationCursor(*this);

            imgui.drawFrame();
        }
    }
};