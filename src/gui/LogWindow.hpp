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
    size_t m_lastLogCount = 0;

    std::vector<int> m_visibleIndices;

    void LogWindowHeader()
    {
        if (ImGui::BeginTable("HeaderBar", 2, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableNextColumn();

            if (ImGui::Button("Clear"))
            {
                Debug::Clear();
                m_visibleIndices.clear();
            }

            ImGui::TableNextColumn();

            float spacing = ImGui::GetStyle().ItemSpacing.x;

            float logsWidth = ImGui::CalcTextSize("Logs").x + ImGui::GetFrameHeight()
                              + ImGui::CalcTextSize(std::to_string(Debug::GetInfoLogCount()).c_str()).x + spacing;

            float warningsWidth = ImGui::CalcTextSize("Warnings").x + ImGui::GetFrameHeight()
                                  + ImGui::CalcTextSize(std::to_string(Debug::GetWarningLogCount()).c_str()).x + spacing;

            float errorsWidth =
                ImGui::CalcTextSize("Errors").x + ImGui::GetFrameHeight() + ImGui::CalcTextSize(std::to_string(Debug::GetErrorLogCount()).c_str()).x;

            float totalWidth = logsWidth + warningsWidth + errorsWidth + spacing * 10;

            // Move cursor BEFORE drawing

            float avail = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - totalWidth);

            ImGui::PushStyleColor(ImGuiCol_CheckMark, Colors::ConcreteGrey_ImGui);
            ImGui::Checkbox("Logs", &showLogs);
            ImGui::SameLine();
            ImGui::Text("(%zu)", Debug::GetInfoLogCount());
            ImGui::PopStyleColor();

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_CheckMark, Colors::HazardYellow_ImGui);
            ImGui::Checkbox("Warnings", &showWarnings);
            ImGui::SameLine();
            ImGui::Text("(%zu)", Debug::GetWarningLogCount());
            ImGui::PopStyleColor();

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_CheckMark, Colors::RustRed_ImGui);
            ImGui::Checkbox("Errors", &showErrors);
            ImGui::SameLine();
            ImGui::Text("(%zu)", Debug::GetErrorLogCount());
            ImGui::PopStyleColor();

            ImGui::EndTable();
        }
    }

    void LogWindowMessages()
    {
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground);

        const auto& logs = Debug::GetLogMessages();
        bool newLogsArrived = logs.size() > m_lastLogCount;
        m_lastLogCount = logs.size();
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
        if (newLogsArrived) ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }

  public:

    void DrawLogsGui()
    {
        LogWindowHeader();
        LogWindowMessages();
    }
};
