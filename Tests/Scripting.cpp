
#include <TLib/Scripting/AngelScript/AngelScript.hpp>
#include <TLib/thirdparty/ImGuiColorTextEdit/TextEditor.h>

#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>

Window   window;
FPSLimit fpslimit;
bool running = true;

TextEditor editor;
Path       openFile;
bool       unsaved = false;

Input::Action actEditorSave = { "Save", { Input::ActionType::KEYBOARD, SDL_SCANCODE_S, SDL_Keymod::KMOD_LCTRL } };
Input::Action actEditorRedo = { "Redo", { Input::ActionType::KEYBOARD, SDL_SCANCODE_Z, SDL_Keymod::KMOD_LCTRL | SDL_Keymod::KMOD_LSHIFT } };

void saveTextEditor()
{
    if (writeToFile(openFile, editor.GetText()))
    {
        tlog::info("Saved");
        unsaved = false;
    }
    else
        tlog::info("Failed to save");
}

void init()
{
    auto lang = TextEditor::LanguageDefinition::CPlusPlus();
    editor.SetLanguageDefinition(lang);
    Path filePath =  R"""(D:\Resources\Dev\CPP\TLib\Tests\assets\scripting.as)""";

    openFile = filePath;
    editor.SetText(readFile(filePath));

    ScriptingEngine se;
    se.create();
    se.generateDocs();
    se.generateStubs();
}

void update(float delta)
{
    ImGui::Begin("Text Editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save", "Ctrl-S"))
            { saveTextEditor(); }
            if (ImGui::MenuItem("Quit", "Alt-F4"))
                running = false;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            bool ro = editor.IsReadOnly();
            if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                editor.SetReadOnly(ro);
            ImGui::Separator();

            if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, !ro && editor.CanUndo()))
                editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Shift-Z", nullptr, !ro && editor.CanRedo()))
                editor.Redo();

            ImGui::Separator();

            if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                editor.Copy();
            if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                editor.Cut();
            if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                editor.Delete();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                editor.Paste();

            ImGui::Separator();

            if (ImGui::MenuItem("Select all", nullptr, nullptr))
                editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Dark palette"))
                editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Retro blue palette"))
                editor.SetPalette(TextEditor::GetRetroBluePalette());
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    auto cpos = editor.GetCursorPosition();
    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
        editor.IsOverwrite() ? "Ovr" : "Ins",
        unsaved ? "*" : " ",
        editor.GetLanguageDefinition().mName.c_str(), openFile.string().c_str());

    if (editor.IsTextChanged()) { unsaved = true; }
    editor.Render("Editor", ImVec2(), true);
    
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
    {
        if      (Input::isActionJustPressed(actEditorSave)) { saveTextEditor(); }
        else if (Input::isActionJustPressed(actEditorRedo)) { editor.Redo();    }
    }

    ImGui::End();
}

void draw(float delta)
{

}

int main()
{
    MyGui imgui;
    Timer deltaTimer;

    WindowCreateParams params;
    params.title = "Window";
    params.size  ={ 1280, 720 };
    window.create(params);
    Renderer::create();
    Renderer2D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    init();

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
        { Input::updateKeyboard(); }
        { Input::updateMouse();    }

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

    return 0;
}