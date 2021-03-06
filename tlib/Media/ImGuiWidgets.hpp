#pragma once

#include <imgui.h>
#include <string>

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