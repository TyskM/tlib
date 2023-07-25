#pragma once

#include <TLib/Media/Platform/Window.hpp>
#include <TLib/NonAssignable.hpp>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

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
        ImGui_ImplSDL2_InitForOpenGL(window, window.getGLContext());
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    bool input(SDL_Event& e)
    {
        return ImGui_ImplSDL2_ProcessEvent(&e);
    }

    void newFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool created() { return window != nullptr; }
};