
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Media/GL/FrameBuffer.hpp>
#include <TLib/Media/RenderTarget.hpp>

const Path     testTexPath = "assets/ship.png";
const Path     fontPath    = "assets/roboto.ttf";
const Vector2f fboTexSize  = {400, 400};
float          totalDelta  = 0.f;

int main()
{
    Window       window;
    MyGui        imgui;
    FPSLimit     fpslimit;
    Timer        deltaTimer;
    RenderTarget rt;

    WindowCreateParams wcp;
    wcp.size = Vector2i(fboTexSize.x * 2, fboTexSize.y * 2);
    window.create(wcp);
    window.setTitle("FBO Test");

    Input::init(window);
    Renderer::create();
    Renderer2D::create();
    rt.create();
    rt.setSize(wcp.size);

    View defaultView;
    defaultView.size   = Vector2f(window.getSize());
    defaultView.center = defaultView.size / 2.f;
    rt.view = defaultView;

    imgui       .create(window);
    deltaTimer  .restart();
    fpslimit    .setFPSLimit(144);
    fpslimit    .setEnabled(true);

    Font font;
    font.loadFromFile(fontPath);

    Texture testTex(testTexPath);
    testTex.setFilter(TextureFiltering::Nearest);

    RenderTarget subRt;
    subRt.create();
    subRt.setSize(fboTexSize.x, fboTexSize.y);
    subRt.view        = rt.view;
    subRt.view.size   = Vector2f(fboTexSize.x, fboTexSize.y);
    subRt.view.center = subRt.view.size / 2.f;

    bool running = true;
    while (running)
    {
        float delta = deltaTimer.restart().asSeconds();
        totalDelta += delta;

        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);
            imgui.input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                auto fbSize = Renderer::getFramebufferSize();
                rt.setSize(fbSize);
                Renderer2D::setView(rt.view);
                Renderer::setViewport(rt.getViewportSizePixels());
            }

            else if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse))    { Input::updateMouse(); }

        Vector2f mworldpos = rt.localToWorldPoint(Vector2f(Input::mousePos));

        debugCamera(rt.view);

        Renderer::clearColor();

        Vector2f pos(abs(sin(totalDelta)) * 20.f, abs(cos(totalDelta)) * 20.f);

        // SubRT Drawing
        Renderer2D::bindRenderTarget(subRt);
            Renderer  ::clearColor(ColorRGBAf::red());
            Renderer2D::drawText("SubRT Drawing", font, mworldpos);
            Renderer2D::drawTexture(testTex, pos);
            Renderer2D::drawCircle(mworldpos, 6.f);
            Renderer2D::render();
        subRt.unbind();

        // RT Drawing
        Renderer2D::bindRenderTarget(rt);
        Renderer  ::clearColor(ColorRGBAf::royalBlue());
        Renderer2D::drawText("RT Drawing", font, mworldpos);
        Renderer2D::drawTexture(testTex, pos);
        Renderer2D::drawCircle(mworldpos, 6.f);

        Renderer2D::drawRenderTarget(subRt, {0, fboTexSize.y, fboTexSize});
        Renderer2D::drawRenderTarget(subRt, {fboTexSize.x, 0, fboTexSize});
        Renderer2D::drawRenderTarget(subRt, {fboTexSize.x, fboTexSize.y, fboTexSize});
        Renderer2D::render();

        imgui.newFrame();
        beginDiagWidgetExt();

        ImGui::Text(fmt::format("Framebuffer Size:   {}", Renderer::getFramebufferSize().toString()).c_str());
        ImGui::Text(fmt::format("Window Size:        {}", window.getSize().toString()).c_str());
        ImGui::Text(fmt::format("Render Target Size: {}", rt.getSize().toString()).c_str());

        ImGui::SliderFloat("Bounds X",      &rt.view.center.x, -3000.f, 3000.f);
        ImGui::SliderFloat("Bounds Y",      &rt.view.center.y, -3000.f, 3000.f);
        ImGui::SliderFloat("Bounds Width",  &rt.view.size.x,   -3000.f, 3000.f);
        ImGui::SliderFloat("Bounds Height", &rt.view.size.y,   -3000.f, 3000.f);
        ImGui::SliderFloat("Zoom X",        &rt.view.zoom.x,    0.01,   8.f);
        ImGui::SliderFloat("Zoom Y",        &rt.view.zoom.y,    0.01,   8.f);

        const Vector2i currentWinSize = window.getSize();
        ImGui::SliderFloat("Viewport X",      &rt.view.viewport.x,      0.f, 1.f);
        ImGui::SliderFloat("Viewport Y",      &rt.view.viewport.y,      0.f, 1.f);
        ImGui::SliderFloat("Viewport Width",  &rt.view.viewport.width,  0.f, 1.f);
        ImGui::SliderFloat("Viewport Height", &rt.view.viewport.height, 0.f, 1.f);

        ImGui::End();
        drawDiagWidget(&fpslimit);

        rt.unbind();
        Renderer2D::drawFinalRenderTarget(rt);
        Renderer2D::render(false, true);

        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}