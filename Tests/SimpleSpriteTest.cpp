
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Timer.hpp>
#include "Common.hpp"

struct TLibTest
{
    Window win;
    Renderer rend;
    Renderer2D rend2d;

    Texture tex;
    Timer deltaClock;

    // FPS stuff
    Timer timer;
    int fpscounter = 0;

    const int spriteCount = 20000;

    void run()
    {
        WindowCreateParams p;
        p.title = "Simple Sprite Test";
        p.size  = { 1280, 720 };
        win.create(p);

        Renderer::create();
        Renderer2D::create();

        tex.loadFromFile("assets/ship.png");

        bool running = true;
        while (running)
        {
            SDL_Event e;
            while (SDL_PollEvent(&e))
            {
                if (e.type == SDL_QUIT) { running = false; }
            }

            rend.clearColor();

            float delta = deltaClock.restart().asSeconds();
            static float time = 0.f;
            time += delta;

            int count = spriteCount;
            int sr = std::ceil(sqrt(count));

            for (int x = 0; x < sr; x++)
            {
                for (int y = 0; y < sr; y++)
                {
                    const float rot = math::deg2rad(sin(time) * x + y);
                    const ColorRGBAf color =
                    {
                        fmodf(sin(time), 1.f) * (y%16),
                        fmodf(cos(time / 2.f) * (x%12), 1.f),
                        fmodf((time)+x+y, 1.f), 1.f
                    };
                    const Rectf rect ={ Vector2f(x, y) * 4.f, Vector2f(32,32) };

                    rend2d.drawTexture(tex, rect, rot, color);

                    --count;
                    if (count == 0) break;
                }
                if (count == 0) break;
            }

            rend2d.render();
            win.swap();

            if (timer.getElapsedTime().asSeconds() > 1.f)
            {
                win.setTitle(String("TLib FPS: ") + std::to_string(fpscounter));
                fpscounter = 0;
                timer.restart();
            }
            ++fpscounter;
        }
    }
};

int main()
{
    TLibTest game;
    game.run();
    return 0;
}