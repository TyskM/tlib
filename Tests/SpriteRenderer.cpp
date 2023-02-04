//
// Created by Ty on 2023-01-29.
//

#include <TLib/Media/Renderer2D.hpp>
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include <TLib/Media/Camera2DDebug.hpp>

#include "Common.hpp"

struct TexTest : GameTest
{
    SharedPtr<Texture> tex;
    Renderer2D      rend2d;
    Vector2f        pos    = { 0.1f, 0.1f };
    Vector2f        srcpos = { 0, 0 };
    Camera2D        view;

    bool  rotationEnabled   = true;
    int   spriteCount       = 30;
    float offset            = 32;

    void create() override
    {
        GameTest::create();
        rend2d.create(renderer);

        tex = makeShared<Texture>();
        tex->loadFromFile("assets/ship.png", TextureFiltering::Nearest);
    }

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);
        imgui.newFrame();
        
        auto bounds = view.getBounds();
        auto size = Renderer::getFramebufferSize();
        view.setBounds(Rectf(bounds.x, bounds.y, size.x, size.y));
        debugCamera(view);
        rend2d.setView(view);

        Vector2f mwpos = view.localToWorldCoords(Input::mousePos);

        rend2d.begin();
        rend2d.clearColor();

        static float time = 0.f;
        time += delta;
        int count = spriteCount;
        int sr = sqrt(spriteCount);

        for (int x = 0; x < sr; x++)
        {
            for (int y = 0; y < sr; y++)
            {
                const float rot = rotationEnabled ? sin(time) * x + y : 0;
                const ColorRGBAf color =
                {
                    fmodf(sin(time), 1.f) * (y%16),
                    fmodf(cos(time / 2.f) * (x%12), 1.f),
                    fmodf((time)+x+y, 1.f), 1
                };
                const Rectf rect ={ Vector2f(x, y) * offset, Vector2f(32,32) };

                rend2d.drawTexture(tex, rect, rot, color);

                --count;
                if (count == 0) break;
            }
        }

        rend2d.drawCircle(mwpos, 12.f);

        beginDiagWidgetExt();
        ImGui::Checkbox    ("Rotation enabled", &rotationEnabled);
        ImGui::SliderInt   ("Sprite count", &spriteCount, 1, 100 * 100);
        ImGui::SliderFloat ("Sprite offset", &offset, 1.f, 128.f, "%.2f");
        ImGui::End();
        drawDiagWidget(&renderer, &fpslimit);

        rend2d.render();
        renderer.render();
        imgui.render();

        window.swap();
        fpslimit.wait();
    }
};

int main()
{
    TexTest game;
    game.create();
    game.run();
    return 0;
}