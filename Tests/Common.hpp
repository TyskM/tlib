//
// Created by Ty on 2023-01-27.
//

#pragma once

#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>

const char* vert_flat = R"""(
        #version 330 core
        layout (location = 0) in vec2 vertex;
        layout (location = 1) in vec4 inColor;
        out vec4 color;

        uniform mat4 projection;

        void main()
        {
            gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
            color = inColor;
        }
        )""";

const char* frag_flat = R"""(
        #version 330 core
        out vec4 FragColor;
        in vec4 color;

        void main()
        {
           FragColor = color;
        }
        )""";

const char* vert_flat3d = R"""(
        #version 330 core
        layout (location = 0) in vec3 vertex;
        layout (location = 1) in vec4 inColor;
        out vec4 color;

        uniform mat4 projection;

        void main()
        {
            gl_Position = projection * vec4(vertex.xyz, 1.0);
            color = inColor;
        }
        )""";

Vector2f getMousePos()
{
    return localToWorldPoint(Vector2f(Input::mousePos), Renderer2D::getView(), Renderer::getFramebufferSize());
}

struct GameTest
{
    Window   window;
    MyGui    imgui;
    FPSLimit fpslimit;
    //const float runDuration = 6.f;
    Timer timer{true};
    Timer deltaTimer;
    bool running = true;

    virtual void create()
    {
        window.create();
        Renderer::create();
        Renderer2D::create();
        imgui.create(window);
        timer.setPaused(false);
        deltaTimer.restart();
        fpslimit.setFPSLimit(60);
        fpslimit.setEnabled(true);
    }

    virtual void mainLoop(float delta)
    {
        //if (timer.getElapsedTime().asSeconds() > runDuration) { running = false; }
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);
            imgui.input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                auto view = Renderer2D::getView();
                view.size = Vector2f(e.window.data1, e.window.data2);
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse))    { Input::updateMouse();    }
    }

    int run()
    {
        running = true;
        while (running)
        { mainLoop(deltaTimer.restart().asSeconds()); }
        return 0;
    }
};