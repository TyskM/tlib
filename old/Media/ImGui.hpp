
// For use with the SDL Renderer
// https://github.com/ocornut/imgui/blob/master/examples/example_sdl_sdlrenderer/main.cpp

#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_sdlrenderer.h>
#include "../NonAssignable.hpp"

#if !SDL_VERSION_ATLEAST(2,0,17)
    #error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

// RAII Wrapper for SDL Renderer
struct ImGuiWrapper : NonAssignable
{
    bool _created = false;

    void create(SDL_Window* window, SDL_Renderer* renderer, ImFontAtlas* font = NULL)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext(font);
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer_Init(renderer);

        _created = true;
    }

    ImGuiWrapper(SDL_Window* window, SDL_Renderer* renderer)
    {
        create(window, renderer);
    }

    // You must call create() before using ImGui
    ImGuiWrapper() { }

    ~ImGuiWrapper()
    {
        if (_created)
        {
            ImGui_ImplSDLRenderer_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void newFrame()
    {
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void render()
    {
        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    }

    void processEvent(SDL_Event* e) { ImGui_ImplSDL2_ProcessEvent(e); }
};