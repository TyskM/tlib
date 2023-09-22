
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/Camera2D.hpp>
#include <TLib/Media/Camera2DDebug.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>

#include <TLib/Media/GUI/Agui.hpp>
#include <TLib/Media/GUI/Backends/TLib/TLib.hpp>
#include <TLib/Media/GUI/FlowLayout.hpp>
#include <TLib/Media/GUI/Widgets/Button/Button.hpp>
#include <TLib/Media/GUI/Widgets/CheckBox/CheckBox.hpp>
#include <TLib/Media/GUI/Widgets/Frame/Frame.hpp>
#include <TLib/Media/GUI/Widgets/DropDown/DropDown.hpp>
#include <TLib/Media/GUI/Widgets/TextField/TextField.hpp>
#include <TLib/Media/GUI/Widgets/RadioButton/RadioButton.hpp>
#include <TLib/Media/GUI/Widgets/RadioButton/RadioButtonGroup.hpp>
#include <TLib/Media/GUI/Widgets/Slider/Slider.hpp>
#include <TLib/Media/GUI/Widgets/TextBox/ExtendedTextBox.hpp>
#include <TLib/Media/GUI/Widgets/Tab/TabbedPane.hpp>
#include <TLib/Media/GUI/Widgets/ScrollPane/ScrollPane.hpp>

namespace agui
{
    struct ImageButton : public Button
    {
    protected:
        agui::Image* defaultImage  = nullptr;
        agui::Image* hoverImage    = nullptr;
        agui::Image* clickImage    = nullptr;
        agui::Image* disabledImage = nullptr;

        virtual void paintBackground(const agui::PaintEvent& paintEvent)
        {
            if (!paintEvent.isEnabled() && disabledImage)
            {
                paintEvent.graphics()->drawNinePatchImage(
                    disabledImage, Vector2i(0, 0), getSize(), 1.0f);
                return;
            }

            switch (getButtonState())
            {
            case DEFAULT:
                if (!defaultImage) { return; }
                paintEvent.graphics()->drawNinePatchImage(
                    defaultImage, Vector2i(0, 0), getSize(), 1.0f);
                return; break;

            case HOVERED:
                if (!hoverImage) { return; }
                paintEvent.graphics()->drawNinePatchImage(
                    hoverImage, Vector2i(0, 0), getSize(), 1.0f);
                return; break;

            case CLICKED:
                if (!clickImage) { return; }
                paintEvent.graphics()->drawNinePatchImage(
                    clickImage, Vector2i(0, 0), getSize(), 1.0f);
                return; break;

            default: break;
            }
        }

        virtual void paintComponent(const agui::PaintEvent& paintEvent)
        {
            resizableText.drawTextArea(paintEvent.graphics(), getFont(),
                getInnerRectangle(), getFontColor(), getAreaText(), getTextAlignment());
        }

    public:
        void setImages(
            agui::Image* defaultImage,
            agui::Image* hoverImage,
            agui::Image* clickImage,
            agui::Image* disabledImage)
        {
            this->defaultImage  = defaultImage;
            this->hoverImage    = hoverImage;
            this->clickImage    = clickImage;
            this->disabledImage = disabledImage;
        }
    };
}

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
    
    UPtr<agui::Frame           > frame;
    UPtr<agui::FlowLayout      > flow;
    UPtr<agui::Button          > button;
    UPtr<agui::CheckBox        > checkBox;
    UPtr<agui::DropDown        > dropDown;
    UPtr<agui::TextField       > textField;
    UPtr<agui::RadioButton     > rButton[3];
    UPtr<agui::RadioButtonGroup> rGroup;
    UPtr<agui::Slider          > slider;
    UPtr<agui::ExtendedTextBox > exTextBox;
    UPtr<agui::TabbedPane      > tabbedPane;
    UPtr<agui::Tab             > tab[3];
    UPtr<agui::ListBox         > listBox;
    UPtr<agui::ScrollPane      > scrollPane;
    UPtr<agui::Button          > scrollButtons[15];
    UPtr<agui::ImageButton     > imgButton;

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

        frame       = makeUnique<agui::Frame           >();
        flow        = makeUnique<agui::FlowLayout      >();
        button      = makeUnique<agui::Button          >();
        checkBox    = makeUnique<agui::CheckBox        >();
        dropDown    = makeUnique<agui::DropDown        >();
        textField   = makeUnique<agui::TextField       >();
        rGroup      = makeUnique<agui::RadioButtonGroup>();
        slider      = makeUnique<agui::Slider          >();
        exTextBox   = makeUnique<agui::ExtendedTextBox >();
        tabbedPane  = makeUnique<agui::TabbedPane      >();
        listBox     = makeUnique<agui::ListBox         >();
        scrollPane  = makeUnique<agui::ScrollPane      >();
        imgButton   = makeUnique<agui::ImageButton     >();

        for (int i = 0; i < std::size(tab); i++)
        { tab[i] = makeUnique<agui::Tab>(); }

        for (int i = 0; i < std::size(scrollButtons); i++)
        { scrollButtons[i] = makeUnique<agui::Button>(); }

        gui->add(frame.get());
        frame->setSize(220, 420);
        frame->setLocation(60, 60);
        frame->setText("Example Frame");
        frame->add(flow.get());

        flow->add(button.get());
        button->setSize(80, 40);
        button->setText("Push Me");
        button->addActionListener(&simpleAL);

        flow->add(checkBox.get());
        checkBox->setAutosizing(true);
        checkBox->setText("Show me a message box");
        checkBox->setCheckBoxAlignment(agui::ALIGN_MIDDLE_LEFT);
        checkBox->addActionListener(&simpleAL);

        std::stringstream ss;
        for (int i = 0; i < std::size(rButton); i++)
        {
            rButton[i] = makeUnique<agui::RadioButton>();
            ss.str("");
            ss.clear();
            ss << "Sample Radio Button ";
            ss << i;

            rGroup->add(rButton[i].get());
            rButton[i]->setAutosizing(true);
            rButton[i]->setText(ss.str());
            flow->add(rButton[i].get());
            rButton[i]->setLocation(0, 30 * i);
            rButton[i]->addActionListener(&simpleAL);
        }

        flow->add(slider.get());
        slider->setSize(100, 36);
        slider->setMaxValue(255);
        slider->setMarkerSize(Vector2i(10, 30));
        slider->addActionListener(&simpleAL);
    }
};

int main()
{
    Window     window;
    MyGui      imgui;
    FPSLimit   fpslimit;
    Timer      deltaTimer;

    window.create();
    window.setTitle("GUI");
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