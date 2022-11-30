#pragma once

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS
#include <imgui_impl_sdl.h>
#include <thirdparty/imgui_impl_sdl.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <SFML/OpenGL.hpp>

namespace MyGui
{
    void init(SDL_Window* sdlwindow)
    {
        auto ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_Init(sdlwindow, NULL);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    void input(SDL_Event& e)
    {
        ImGui_ImplSDL2_ProcessEvent(&e);
    }

    void newFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void render()
    {
        // TODO: might need to push and pop gl state
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}