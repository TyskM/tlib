
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Media/Box2D.hpp>
#include "Common.hpp"

float scale = 6.f;

template <typename T>
T toPhysCoords(const T& v)
{ return v / scale; }

template <typename T>
T toPixCoords(const T& v)
{ return v * scale; }

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

    // World
    const int32_t velocityIterations = 6;
    const int32_t positionIterations = 2;
    TLibBox2DDebugDraw dbgDraw;
    dbgDraw.scale = scale;
    b2World world({0.f, 10.f});
    world.SetDebugDraw(&dbgDraw);
    dbgDraw.SetFlags(dbgDraw.allBit);
    
    // Static
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(50.0f, 25.0f);
    b2Body* groundBody = world.CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 5.0f);
    groundBody->CreateFixture(&groundBox, 0.0f);

    // Dynamic
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(25.0f, 4.0f);
    b2Body* body = world.CreateBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;
    body->CreateFixture(&fixtureDef);

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
        if (!(io.WantCaptureMouse))    { Input::updateMouse(); }

        auto view = Renderer2D::getView();
        debugCamera(view);
        Renderer2D::setView(view);

        Vector2f mousePos = getMousePos();

        if (Input::isMouseJustPressed(Input::MOUSE_LEFT))
        {
            bodyDef.position.Set(toPhysCoords(mousePos.x), toPhysCoords(mousePos.y));
            b2Body* body = world.CreateBody(&bodyDef);
            body->CreateFixture(&fixtureDef);
        }

        world.Step(delta, velocityIterations, positionIterations);

        Renderer::clearColor();
        world.DebugDraw();
        Renderer2D::render();

        imgui.newFrame();
        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}