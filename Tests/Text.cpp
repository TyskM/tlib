
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include <TLib/Media/Camera2DDebug.hpp>
#include "Common.hpp"

struct TextTest : GameTest
{
    SDFFont font;

    String text =
R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit.
Suspendisse consequat urna odio, nec pellentesque augue suscipit at.
Maecenas sit amet est vel massa consectetur cursus. Curabitur vel interdum ipsum.
Nam sed purus eget augue rutrum hendrerit. Fusce malesuada lobortis leo a semper.
Sed a vulputate nibh. Morbi iaculis elit purus, non suscipit metus tincidunt quis.
Pellentesque semper orci quis massa vestibulum, in dapibus risus volutpat.
Proin sollicitudin et tortor non pulvinar. Sed tempus purus quam, quis faucibus dui maximus vitae.
Maecenas aliquam consequat odio, quis mattis diam hendrerit id. Praesent vitae ipsum sed sapien consectetur condimentum.
Nullam molestie nisl vitae dapibus tristique. Duis nec lacus mi. Proin id est eget odio facilisis egestas.
Fusce sodales est scelerisque erat iaculis elementum.
ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz123456789!@#$%^&*()-=_+\|,./;'[]<>?:"{}`~
)";

    void create() override
    {
        GameTest::create();
        window.setTitle("Text Test");
        font.loadFromFile("assets/arial.ttf", 24, 0, 256);
        font.getAtlas().writeToFile("atlas.png");
    }

    void mainLoop(float delta) override
    {
        GameTest::mainLoop(delta);
        imgui.newFrame();

        auto view = Renderer2D::getView();
        debugCamera(view);
        Renderer2D::setView(view);

        Vector2f mwpos = view.localToWorldCoords(Input::mousePos);

        Renderer::clearColor();

        static float time = 0.f;
        time += delta;

        Vector2f textPos = {50, 50};
        Vector2f fontSize = font.calcTextSize(text);
        Renderer2D::drawTexture(font.getAtlas(), {textPos - Vector2f(0, 20) - Vector2f(0, font.getAtlas().getSize().y), Vector2f(font.getAtlas().getSize())});
        Renderer2D::drawCircle(textPos, 3.f, 1, ColorRGBAf::red(), true);
        Renderer2D::drawText(text, font, textPos);
        Renderer2D::drawCircle(mwpos, 12.f);
        Renderer2D::render();

        drawDiagWidget(&fpslimit);

        imgui.render();

        window.swap();
        fpslimit.wait();
    }
};

int main()
{
    TextTest game;
    game.create();
    game.run();
    return 0;
}