
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include "Common.hpp"

View camera;

const int        rectCount     = 32;
const float      rectSize      = 128.f;
const ColorRGBAf rectColors[2] ={ColorRGBAf::magenta(), ColorRGBAf::darkMagenta()};

void resetCamera()
{
    Renderer2D::resetView();
    camera = Renderer2D::getView();
}

int main()
{
    Window     window;
    MyGui      imgui;
    FPSLimit   fpslimit;
    Timer      deltaTimer;

    window.create();
    window.setTitle("Camera2D");
    Renderer::create();
    Renderer2D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    resetCamera();

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
                auto view = Renderer2D::getView();
                view.size = Vector2f(e.window.data1, e.window.data2);
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        Renderer::clearColor();

        // Update/Draw Here
        static bool currentColor = false;
        for (size_t x = 0; x < rectCount; x++)
        {
            for (size_t y = 0; y < rectCount; y++)
            {
                
                Rectf rect(x * rectSize, y * rectSize, rectSize, rectSize);
                Renderer2D::drawRect(rect, 0.f, true, rectColors[currentColor]);
                currentColor = !currentColor;
            }
            currentColor = !currentColor;
        }

        const Vector2f mouseLocalPos = Vector2f(Input::mousePos);
        const Vector2f mouseWorldPos = getMousePos();
        Renderer2D::drawCircle(mouseLocalPos, 3.f, 1, ColorRGBAf::red(),       true);
        Renderer2D::drawCircle(mouseWorldPos, 3.f, 1, ColorRGBAf::steelBlue(), true);

        debugCamera(camera);
        Renderer2D::setView(camera);
        Renderer2D::render();

        imgui.newFrame();

        beginDiagWidgetExt();

        ImGui::SliderFloat("Center X", &camera.center.x,      -3000.f, 3000.f);
        ImGui::SliderFloat("Center Y", &camera.center.y,      -3000.f, 3000.f);
        ImGui::SliderFloat("Size X",   &camera.size.x,        -3000.f, 3000.f);
        ImGui::SliderFloat("Size Y",   &camera.size.y,        -3000.f, 3000.f);
        ImGui::SliderFloat("Zoom X",   &camera.zoom.x,         0.01,  8.f);
        ImGui::SliderFloat("Zoom Y",   &camera.zoom.y,         0.01,  8.f);
        ImGui::SliderFloat("Rotation", &camera.rotation,      -glm::pi<float>() * 4, glm::pi<float>() * 4);

        const Vector2i currentWinSize = window.getSize();
        ImGui::SliderFloat("Viewport X",      &camera.viewport.x,      0,      1.f);
        ImGui::SliderFloat("Viewport Y",      &camera.viewport.y,      0,      1.f);
        ImGui::SliderFloat("Viewport Width",  &camera.viewport.width,  0.01f,  1.f);
        ImGui::SliderFloat("Viewport Height", &camera.viewport.height, 0.01f,  1.f);

        if (ImGui::Button("Reset"))
        {
            resetCamera();
        }

        ImGui::End();
        
        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}