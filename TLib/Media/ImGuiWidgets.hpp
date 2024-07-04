#pragma once

#include <TLib/Media/ImGui.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/SysQuery.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Containers/Pair.hpp>
#include <string>
#include <format>
#include <magic_enum.hpp>

namespace ImGui
{
    void Image(Texture&    tex,
         const Vector2f&   size,
         const Rectf&      srcRect,
         const ColorRGBAf& tintColor   = ColorRGBAf::white(),
         const ColorRGBAf& borderColor = ColorRGBAf::transparent())
    {
        auto handle = (ImTextureID)tex.handle();
        auto uv = Renderer2D::getTextureUVs(tex, srcRect);
        ImGui::Image(handle, { size.x, size.y },
            { uv.first.x, uv.first.y }, { uv.second.x, uv.second.y },
            { tintColor.r, tintColor.g, tintColor.b, tintColor.a },
            { borderColor.r, borderColor.g, borderColor.b, borderColor.a });
    }

    void Image(Texture&    tex,
         const Vector2f&   size,
         const ColorRGBAf& tintColor   = ColorRGBAf::white(),
         const ColorRGBAf& borderColor = ColorRGBAf::transparent())
    {

        auto handle = (ImTextureID)tex.handle();
        ImGui::Image(handle, { size.x, size.y }, {0,0}, {1,1},
            { tintColor.r, tintColor.g, tintColor.b, tintColor.a },
            { borderColor.r, borderColor.g, borderColor.b, borderColor.a });
    }
}

void debugCamera(View& view, const float minZoom = 0.1f, const float maxZoom = 40.f, const float zoomIncr = 0.15f)
{
    static bool dragging = false;

    // Dragging
    if (Input::isMousePressed(Input::MOUSE_MIDDLE))
    {
        view.center.x -= Input::mouseDelta.x / view.zoom.x;
        view.center.y -= Input::mouseDelta.y / view.zoom.y;
    }

    // Zooming
    if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_DOWN))
    {
        view.zoom.x -= zoomIncr * view.zoom.x;
        view.zoom.y -= zoomIncr * view.zoom.y;
    } else if (Input::isMouseJustPressed(Input::MOUSE_WHEEL_UP))
    {
        view.zoom.x += zoomIncr * view.zoom.x;
        view.zoom.y += zoomIncr * view.zoom.y;
    }
    view.zoom.x = std::clamp(view.zoom.x, minZoom, maxZoom);
    view.zoom.y = std::clamp(view.zoom.y, minZoom, maxZoom);
}

// Returns { bool wasJustChanged, EnumType newValue }
template <typename EnumType>
Pair<bool, EnumType> imguiEnumCombo(const char* name, EnumType value)
{
    Pair<bool, EnumType> ret = { false, value };
    auto cont = magic_enum::enum_values<EnumType>();
    if (ImGui::BeginCombo(name, magic_enum::enum_name(value).data()))
    {
        for (auto& v : cont)
        {
            bool selected = (value == v);

            if (ImGui::Selectable(magic_enum::enum_name(v).data(), selected))
            { ret = { true, v }; }

            if (selected)
            { ImGui::SetItemDefaultFocus(); }
        }
        ImGui::EndCombo();
    }
    return ret;
}

String diagWindowName = "Diagnostics";
// End with ImGui::End()
void beginDiagWidgetExt(bool* p_open = NULL, ImGuiWindowFlags flags = 0)
{
    ImGui::Begin(diagWindowName.c_str(), p_open);
}

void drawDiagWidget(FPSLimit* fpslimit = nullptr, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
{
    float delta = ImGui::GetIO().DeltaTime;
    const auto meminfo  = sysq::getGlobalMemInfo();
    const auto meminfop = sysq::getThisProcessMemUsage();
    const auto vmeminfo = Renderer::getVideoMemoryInfo();

    static float snapTimeSecs = 0.66f;
    static float snapTimeCurr = 0.f;

    static int fpscounter = 0;
    static int fps = 0;
    static Timer t;
    if (t.getElapsedTime().asSeconds() > 1.f)
    {
        fps = fpscounter;
        fpscounter = 0;
        t.restart();
    }
    ++fpscounter;

    beginDiagWidgetExt(p_open, flags);

    if (fpslimit)
    {
        int temp = fpslimit->getFPSLimit();
        ImGui::Checkbox("FPS Limit Enabled", &fpslimit->enabled);
        if (ImGui::SliderInt("FPS Limit", &temp, 30, 240))
        { fpslimit->setFPSLimit(temp); }
    }

    size_t drawCalls = Renderer::getDrawCount();
    Renderer::resetDrawCount();


    //VSyncMode vsyncMode = renderer->getVSync();
    //auto cont = magic_enum::enum_values<VSyncMode>();
    //if (ImGui::BeginCombo("VSync", magic_enum::enum_name(vsyncMode).data()))
    //{
    //    for (auto& v : cont)
    //    {
    //        bool selected = (vsyncMode == v);
    //        if (ImGui::Selectable(magic_enum::enum_name(v).data(), selected))
    //        { renderer->setVSync(v); }
    //        if (selected)
    //        { ImGui::SetItemDefaultFocus(); }
    //    }
    //    ImGui::EndCombo();
    //}

    ImGui::Text(fmt::format("Draw Calls         : {}", drawCalls).c_str());
    ImGui::Text(            "Delta              : %f"   , delta);
    ImGui::Text(fmt::format("FPS                : {}"   , fps).c_str());
    ImGui::Text(fmt::format("CPU Usage          : {}%"  , sysq::getThisProcessCPUUsage()).c_str());
    ImGui::Text(fmt::format("Phys RAM Used      : {} MB", sysq::bytesToMb(meminfop.workingSetSize)).c_str());
    ImGui::Text(fmt::format("Virt RAM Used      : {} MB", sysq::bytesToMb(meminfop.privateUsage)).c_str());
    ImGui::Text(fmt::format("Phys RAM Avail     : {} MB", sysq::bytesToMb(meminfo.availPhysical)).c_str());
    ImGui::Text(fmt::format("Phys RAM Total     : {} MB", sysq::bytesToMb(meminfo.totalPhysical)).c_str());
    ImGui::Text(fmt::format("Virt RAM Avail     : {} MB", sysq::bytesToMb(meminfo.availVirtual)).c_str());
    ImGui::Text(fmt::format("Virt RAM Total     : {} MB", sysq::bytesToMb(meminfo.totalVirtual)).c_str());
    ImGui::Text(fmt::format("VRAM Total         : {} MB", sysq::kbToMb(vmeminfo.total)).c_str());
    ImGui::Text(fmt::format("VRAM Total Avail   : {} MB", sysq::kbToMb(vmeminfo.totalAvailable)).c_str());
    ImGui::Text(fmt::format("VRAM Current Avail : {} MB", sysq::kbToMb(vmeminfo.currentAvailable)).c_str());

    ImGui::End();
}

// https://github.com/ocornut/imgui/blob/master/imgui_demo.cpp
// Usage:
//  ImGuiLog my_log;
//  my_log.write("Hello world\n");
//  my_log.draw("title");
struct ImGuiLog
{
    ImGuiTextBuffer textBuffer;
    ImVector<int>   lineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool            autoScroll;  // Keep scrolling if already at the bottom.

    ImGuiLog()
    {
        autoScroll = true;
        clear();
    }

    void clear()
    {
        textBuffer.clear();
        lineOffsets.clear();
        lineOffsets.push_back(0);
    }

    inline void writeln(const std::string& text) { write(text + '\n'); }

    void write(const std::string& text)
    {
        int old_size = textBuffer.size();
        textBuffer.append(text.c_str());
        for (int new_size = textBuffer.size(); old_size < new_size; old_size++)
            if (textBuffer[old_size] == '\n')
                lineOffsets.push_back(old_size + 1);
    }

    void draw(const char* title, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
    {
        if (!ImGui::Begin(title, p_open, flags))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &autoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool shouldclear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");

        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        if (shouldclear) clear();
        if (copy) ImGui::LogToClipboard();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char* buf = textBuffer.begin();
        const char* buf_end = textBuffer.end();

        ImGuiListClipper clipper;
        clipper.Begin(lineOffsets.Size);
        while (clipper.Step())
        {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
            {
                const char* line_start = buf + lineOffsets[line_no];
                const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                ImGui::TextUnformatted(line_start, line_end);
            }
        }
        clipper.End();

        ImGui::PopStyleVar();

        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
};
