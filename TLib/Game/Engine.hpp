#pragma once

/*

#include <Game/Engine.hpp>

using game::renderer;
using game::imgui;
using game::win;
using game::stateMan;
using game::texMan;

struct PlayState : GameState
{
    void input(const InputEvent& ev) override
    {

    }

    void update(float delta) override
    {
        imgui.newFrame();
    }

    void draw(float delta) override
    {
        renderer.begin();
        renderer.clearColor();



        renderer.render();
        imgui.render();
    }
};

int main()
{
    game::init();
    PlayState ps;
    game::mainLoop(ps);
    return 0;
}

*/

#include <TLib/Timer.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Resource/Texture.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/ImGui.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Helpers.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Game/TextureManager.hpp>
#include <TLib/Game/StateMan.hpp>

using InputEvent = SDL_Event;

struct GameState : State
{
    virtual void input(const SDL_Event& e) { }
    virtual void update(float delta) { }
    virtual void draw(float delta) { }
};

namespace game
{
    Window              win;
    Renderer            renderer;
    MyGui               imgui;
    Timer               dtTimer;
    FPSLimit            fpslimit;
    StateMan<GameState> stateMan;
    TextureManager      texMan;

    void initNoGL(const WindowCreateParams& params = WindowCreateParams())
    {
        win.create(params);
        renderer.create();
        renderer.setVSync(VSyncMode::Enabled);
        imgui.create(win);
    }

    void init(const WindowCreateParams& params = WindowCreateParams())
    {
        WindowCreateParams p = params;
        p.flags |= WindowFlags::OpenGL;
        initNoGL(p);
    }

    void mainLoop(GameState& state)
    {
        ASSERT(win.created() && renderer.created());

        stateMan.pushState(&state);
        dtTimer.restart();

        bool running = true;
        while (running)
        {
            auto& io = ImGui::GetIO();
            if (!(io.WantCaptureKeyboard || io.WantCaptureMouse)) { Input::update(); }

            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) { running = false; }

                imgui.input(e);
                if (io.WantCaptureKeyboard || io.WantCaptureMouse) { continue; }
                Input::input(e);
                stateMan.getState()->input(e);
            }
            float dt = static_cast<float>(dtTimer.restart().asSeconds());
            stateMan.getState()->update(dt);
            stateMan.getState()->draw(dt);
            glState.reset();
            win.swap();
            fpslimit.wait();
        }
    }
}