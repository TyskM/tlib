
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Containers/Variant.hpp>

Window   window;
FPSLimit fpslimit;

namespace p2d
{
    struct CircleShape
    {
        float    radius = 0.f;
        Vector2f offset;
    };

    using Shape = Variant<CircleShape>;

    struct Body
    {
        Vector2f position;
        Vector2f velocity;
        float    rotation        = 0.f;
        float    angularVelocity = 0.f;
        Shape    shape;
    };

    static bool intersectsCircleCircle(
        Body& bodyA, CircleShape& shapeA,
        Body& bodyB, CircleShape& shapeB)
    {
        float r = shapeA.radius + shapeB.radius;
        return r < bodyA.position.distanceTo(bodyB.position);
    }

    static bool intersects(Body& a, Body& b)
    {
        if (a.shape.is<CircleShape>() && b.shape.is<CircleShape>())
        {
            return intersectsCircleCircle(a, a.shape.get<CircleShape>(),
                                          b, b.shape.get<CircleShape>());
        }
    }

    static void solve()
    {

    }
}

static void init()
{

}

static void shutdown()
{

}

static void update(float delta)
{
    Vector2f mouseLocalPos = Vector2f(Input::mousePos);
    Vector2f mouseWorldPos = localToWorldPoint(mouseLocalPos, Renderer2D::getView(), Renderer::getFramebufferSize());
}

static void draw(float delta)
{

}

int main()
{
    MyGui imgui;
    Timer deltaTimer;

    WindowCreateParams params;
    params.title = "Window";
    params.size  = {1280, 720};
    window.create(params);
    Input::init(window);
    Renderer::create();
    Renderer2D::create();

    imgui       .create(window);
    deltaTimer  .restart();
    fpslimit    .setFPSLimit(144);
    fpslimit    .setEnabled(true);

    init();

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

        imgui.newFrame();
        update(delta);
        Renderer::clearColor();
        draw(delta);
        Renderer2D::render();

        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    shutdown();

    return 0;
}
