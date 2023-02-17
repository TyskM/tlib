
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include <TLib/Media/Camera2DDebug.hpp>
#include "Common.hpp"

struct SpriteTest : GameTest
{
    Texture  tex;
    Font     font;

    bool  rotationEnabled   = true;
    int   spriteCount       = 30;
    float offset            = 32;

    void create() override
    {
        GameTest::create();
        tex.loadFromFile("assets/ship.png");
        tex.setFilter(TextureFiltering::Nearest);
        font.loadFontSdf("assets/arial.ttf");
    }

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);
        imgui.newFrame();
        
        auto view = rend2d.getView();
        debugCamera(view);
        rend2d.setView(view);

        Vector2f mwpos = view.localToWorldCoords(Input::mousePos);

        rend2d.clearColor();

        static float time = 0.f;
        time += delta;
        int count = spriteCount;
        int sr = std::ceil(sqrt(spriteCount));

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
                const Rectf rect = { Vector2f(x, y) * offset, Vector2f(32,32) };

                rend2d.drawTexture(tex, rect, 0, color, rot);
                --count;
                if (count == 0) break;
            }
            if (count == 0) break;
        }

        rend2d.drawCircle(mwpos, 12.f);
        rend2d.drawText("Hello world!", font, { 50, 50 });

        rend2d.render();
        renderer.render();

        beginDiagWidgetExt();
        ImGui::Checkbox    ("Rotation enabled", &rotationEnabled);
        ImGui::SliderInt   ("Sprite count", &spriteCount, 1, 20000);
        ImGui::SliderFloat ("Sprite offset", &offset, 1.f, 128.f, "%.2f");
        ImGui::End();
        drawDiagWidget(&renderer, &fpslimit);

        imgui.render();

        window.swap();
        fpslimit.wait();
    }
};

int main()
{
    SpriteTest game;
    game.create();
    game.run();
    return 0;
}