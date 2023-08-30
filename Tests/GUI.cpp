
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>

#include <TLib/Media/GUI/Agui.hpp>
#include <TLib/Media/GUI/Backends/TLib/TLib.hpp>
#include <TLib/Media/GUI/FlowLayout.hpp>
#include <TLib/Media/GUI/Widgets/Button/Button.hpp>
#include <TLib/Media/GUI/Widgets/CheckBox/CheckBox.hpp>

struct UI
{
    class SimpleActionListener : public agui::ActionListener
    {
    public:
        virtual void actionPerformed(const agui::ActionEvent& evt)
        {
            tlog::info("Wow, you did something!!");
        }
    };


    UPtr<agui::Gui>          gui             = NULL;
    UPtr<agui::TLibInput>    inputHandler    = NULL;
    UPtr<agui::TLibGraphics> graphicsHandler = NULL;
    UPtr<agui::Font>         defaultFont     = NULL;

    SimpleActionListener simpleAL;
    agui::FlowLayout flow;
    agui::Button button;
    agui::CheckBox checkBox;

    void init()
    {
        agui::Image::setImageLoader(new agui::TLibImageLoader());
        agui::Font::setFontLoader(new agui::TLibFontLoader());
        inputHandler    = makeUnique<agui::TLibInput>();
        graphicsHandler = makeUnique<agui::TLibGraphics>();
        agui::Color::setPremultiplyAlpha(false);
        gui = makeUnique<agui::Gui>();
        gui->setInput(inputHandler.get());
        gui->setGraphics(graphicsHandler.get());

        defaultFont.reset(agui::Font::load("assets/roboto.ttf", 16));
        agui::Widget::setGlobalFont(defaultFont.get());

        gui->add(&flow);

        flow.add(&button);
        button.setSize(80, 40);
        button.setText("Push Me");
        button.addActionListener(&simpleAL);

        flow.add(&checkBox);
        checkBox.setAutosizing(true);
        checkBox.setText("Show me a message box");
        checkBox.setCheckBoxAlignment(agui::ALIGN_MIDDLE_LEFT);
        checkBox.addActionListener(&simpleAL);
    }
};

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

    UI ui;
    ui.init();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

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

            ui.inputHandler->processEvent(e);
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        ui.gui->logic();

        Renderer::clearColor();
        ui.gui->render();
        Renderer2D::render();

        imgui.newFrame();
        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}