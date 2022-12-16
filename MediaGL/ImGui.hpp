#pragma once

#include "Window.hpp"
#include "../NonAssignable.hpp"
#include "FrameBuffer.hpp"
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>

struct MyGui : NonAssignable
{
    Renderer* renderer = nullptr;

    MyGui(Renderer& window) { create(window); }
    MyGui() { }

    ~MyGui()
    {
        if (created())
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL2_Shutdown();
        }
    }

    void create(Renderer& renderer)
    {
        this->renderer = &renderer;

        auto ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);
        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForOpenGL(renderer.window->window, renderer.glContext);
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
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool created() { return renderer != nullptr; }
};