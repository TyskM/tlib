
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>

int main()
{
    Window     window;
    Renderer   renderer;
    Renderer2D rend2d;
    MyGui      imgui;
    FPSLimit   fpslimit;
    Timer      deltaTimer;

    window      .create();
    renderer    .create();
    rend2d      .create(renderer);
    imgui       .create(window);
    deltaTimer  .restart();
    fpslimit    .setFPSLimit(144);
    fpslimit    .setEnabled(true);

    bool running = true;
    while (running)
    {
        float delta = deltaTimer.restart().asSeconds();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);
            imgui.input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                auto view = rend2d.getView();
                view.setBoundsSize(Vector2f(e.window.data1, e.window.data2));
                rend2d.setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        // Update/Draw Here
    }

    return 0;
}