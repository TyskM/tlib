
#include <TLib/DataStructures.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Frustum.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include <TLib/Media/Camera2DDebug.hpp>
#include "Common.hpp"

struct TextTest : GameTest
{
    Font font;

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
)";

    void create() override
    {
        GameTest::create();
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

        rend2d.drawCircle(mwpos, 12.f);
        rend2d.drawText(text, font, { 50, 50 });

        rend2d.render();
        renderer.render();

        beginDiagWidgetExt();
        ImGui::End();
        drawDiagWidget(&renderer, &fpslimit);

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