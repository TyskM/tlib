
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
    MyGui      imgui;
    FPSLimit   fpslimit;
    Timer      deltaTimer;

    window.create();
    window.setTitle("Minimal Example");
    Renderer::create();
    Renderer2D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    Texture atlas("assets/ui/buttons.png");
    atlas.setFilter(TextureFiltering::Nearest);
    Rectf buttonDefault = {53,  202, 49, 49};
    Rectf buttonHovered = {2,   202, 49, 49};
    Rectf buttonClicked = {104, 104, 49, 45};

    float left   = 6;
    float right  = 6;
    float bottom = 10;
    float top    = 6;
    Vector2f tl = {20, 20};
    Vector2f br = tl + Vector2f{100, 100};

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
                view.setBoundsSize(Vector2f(e.window.data1, e.window.data2));
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        Renderer::clearColor();

        // Update/Draw Here
        Vector2f mousePos = Renderer2D::getView().localToWorldCoords(Input::mousePos);

        if (Input::isMousePressed(Input::MOUSE_RIGHT))
        { br = mousePos; }
        else if (Input::isMousePressed(Input::MOUSE_MIDDLE))
        { tl = mousePos; }

        Rectf rect = Rectf::fromLTRB(tl, br);
        bool hovered = rect.contains(mousePos);
        bool clicked = (hovered && Input::isMousePressed(Input::MOUSE_LEFT));

        if (clicked)
            Renderer2D::drawNinePatchTex(atlas, buttonClicked, rect, left, right, top, bottom);
        else if (hovered)
            Renderer2D::drawNinePatchTex(atlas, buttonHovered, rect, left, right, top, bottom);
        else
            Renderer2D::drawNinePatchTex(atlas, buttonDefault, rect, left, right, top, bottom);

        Renderer2D::render();

        imgui.newFrame();
        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}