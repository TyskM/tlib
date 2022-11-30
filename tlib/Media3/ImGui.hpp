#pragma once

#include "Window.hpp"
#include "../NonAssignable.hpp"
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

struct MyGui : NonAssignable
{
    Window* window = nullptr;

    MyGui(Window& window) { create(window); }
    MyGui() { }

    ~MyGui()
    {
        if (created())
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
        }
    }

    void create(Window& window)
    {
        this->window = &window;

        auto ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForOpenGL(window, window.glContext);
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
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool created() { return window != nullptr; }
};