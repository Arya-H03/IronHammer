#pragma once
#include <imgui.h>
#include <string>
#include "core/utils/Colors.h"
#include "core/utils/Debug.h"

class LogWindow
{
  private:

    bool showLogs = true;
    bool showWarnings = true;
    bool showErrors = true;
    size_t m_selectedIndex;

    std::vector<int> m_visibleIndices;

    void LogWindowHeader()
    {
        if (ImGui::BeginTable("HeaderBar", 2, ImGuiTableFlags_SizingStretchProp))
        {
            // ===== Left Column =====
            ImGui::TableNextColumn();

            if (ImGui::Button("Clear"))
            {
                Debug::Clear();
                m_visibleIndices.clear();
            }

            // ===== Right Column =====
            ImGui::TableNextColumn();

            // Calculate total width of right-side controls
            float spacing = ImGui::GetStyle().ItemSpacing.x;

            float logsWidth = ImGui::CalcTextSize("Logs").x + ImGui::GetFrameHeight();
            float warningsWidth = ImGui::CalcTextSize("Warnings").x + ImGui::GetFrameHeight();
            float errorsWidth = ImGui::CalcTextSize("Errors").x + ImGui::GetFrameHeight();

            float totalWidth = logsWidth + warningsWidth + errorsWidth + spacing * 4;

            // Move cursor so controls are right-aligned
            float columnWidth = ImGui::GetColumnWidth();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + columnWidth - totalWidth);

            // Logs Toggle
            ImGui::PushStyleColor(ImGuiCol_CheckMark, Colors::ConcreteGrey_ImGui);
            ImGui::Checkbox("Logs", &showLogs);
            ImGui::PopStyleColor();

            ImGui::SameLine();

            // Warnings Toggle
            ImGui::PushStyleColor(ImGuiCol_CheckMark, Colors::HazardYellow_ImGui);
            ImGui::Checkbox("Warnings", &showWarnings);
            ImGui::PopStyleColor();

            ImGui::SameLine();

            // Errors Toggle
            ImGui::PushStyleColor(ImGuiCol_CheckMark, Colors::RustRed_ImGui);
            ImGui::Checkbox("Errors", &showErrors);
            ImGui::PopStyleColor();

            ImGui::EndTable();
        }
    }

    void LogWindowMessages()
    {
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);

        const auto& logs = Debug::GetLogMessages();
        m_visibleIndices.clear();
        for (int i = 0; i < logs.size(); ++i)
        {
            const auto& log = logs[i];
            if (log.logType == LogType::Info && !showLogs) continue;
            if (log.logType == LogType::Warning && !showWarnings) continue;
            if (log.logType == LogType::Error && !showErrors) continue;
            m_visibleIndices.push_back(i);
        }

        ImGuiListClipper clipper;
        clipper.Begin((int) m_visibleIndices.size());
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                const LogMessage& log = logs[m_visibleIndices[i]];
                ImGui::Text("%s", log.time.c_str());
                ImGui::SameLine();
                ImGui::TextColored(log.color, "%s", log.message.c_str());

                std::string label = log.shortFilePath + ": " + std::to_string(log.lineNumber) + "##" + std::to_string(m_visibleIndices[i]);
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::LinkBlue_ImGui);
                if (ImGui::Selectable(label.c_str(), false))
                {
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    std::string command = "zed " + log.fullFilePath + ":" + std::to_string(log.lineNumber);
                    std::system(command.c_str());
                }
                ImGui::PopStyleColor();
                ImGui::Separator();
            }
        }
        ImGui::EndChild();
    }

  public:

    void DrawLogsGui()
    {
        LogWindowHeader();
        LogWindowMessages();
    }
};
