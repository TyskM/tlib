
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include "Common.hpp"

struct SpriteTest : GameTest
{
    Texture tex;
    Font    sdfFont;
    Font    bitmapFont;

    bool  rotationEnabled   = true;
    int   spriteCount       = 30;
    float offset            = 32;

    void create() override
    {
        GameTest::create();
        window.setTitle("Sprite Renderer");
        tex.loadFromFile("assets/ship.png");
        tex.setFilter(TextureFiltering::Nearest);
        sdfFont.loadFromFile("assets/roboto.ttf", 24);
        bitmapFont.loadFromFile("assets/roboto.ttf", 24, 0, 128, FontRenderMode::Normal);
    }

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);
        imgui.newFrame();
        
        auto view = Renderer2D::getView();
        debugCamera(view);
        Renderer2D::setView(view);

        Vector2f mwpos = getMousePos();

        Renderer::clearColor();

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
                    fmodf((time)+x+y, 1.f), 1.f
                };
                const Rectf rect = { Vector2f(x, y) * offset, Vector2f(32,32) };

                Renderer2D::drawTexture(tex, rect, rot, color, 0);
                --count;
                if (count == 0) break;
            }
            if (count == 0) break;
        }

        Renderer2D::drawCircle(mwpos, 12.f);
        Renderer2D::drawCircle(mwpos + Vector2f(20, 20), 12.f);
        Renderer2D::drawCircle({0, 0}, 6.f);
        Renderer2D::drawRect({ mwpos, Vector2f(20, 20) });
        Renderer2D::drawText("Hello world!", sdfFont,    { 50, 50 });
        Renderer2D::drawText("Hello world!", bitmapFont, { 50, 50 + float(sdfFont.newLineHeight()) });

        Renderer2D::render();

        float tempWidth = Renderer2D::getSDFTextWidth();
        float tempEdge  = Renderer2D::getSDFTextEdge();

        beginDiagWidgetExt();
        ImGui::Checkbox    ("Rotation enabled", &rotationEnabled);
        ImGui::SliderInt   ("Sprite count", &spriteCount, 1, 20000);
        ImGui::SliderFloat ("Sprite offset", &offset, 1.f, 128.f, "%.2f");

        if (ImGui::SliderFloat("Text Width", &tempWidth, 0.0f, 1.f))
        { Renderer2D::setSDFTextWidth(tempWidth); }

        if (ImGui::SliderFloat("Text Edge", &tempEdge, 0.0f, 1.f))
        { Renderer2D::setSDFTextEdge(tempEdge); }

        ImGui::End();
        drawDiagWidget(&fpslimit);

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