#pragma once
#include "editor/EditorContext.h"
#include "gui/LogWindow.hpp"
#include <imgui.h>

class BottomPanel
{
  private:

    EditorContext& m_editorContext;
    LogWindow m_logWindow;

    void DrawLogTab()
    {
        if (ImGui::BeginTabItem("Logs"))
        {
            m_logWindow.DrawLogsGui();
            ImGui::EndTabItem();
        }
    }

    void DrawEntityTemplateTab()
    {
        if (!m_editorContext.entityTemplateManager) return;

        if (ImGui::BeginTabItem("Entity Templates"))
        {
            for (auto& [name, entityTemplate] : m_editorContext.entityTemplateManager->GetEntityTemplates())
            {
                if (ImGui::Selectable(name.c_str()))
                {
                    m_editorContext.inspector.InspectEntityTemplate(*entityTemplate);
                }

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip))
                {
                    ImGui::SetDragDropPayload("ENTITY_TEMPLATE", name.c_str(), name.size() + 1);
                    ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                    ImGui::BeginTooltip();
                    ImGui::Text("%s", name.c_str());
                    ImGui::EndTooltip();
                    ImGui::EndDragDropSource();
                }
            }
            ImGui::EndTabItem();
        }
    }

  public:

    BottomPanel(EditorContext& editorContext) : m_editorContext(editorContext) { }

    void Draw()
    {
        ImGui::SetNextWindowPos(ImVec2((float) m_editorContext.layout.Log_X, (float) m_editorContext.layout.Log_Y));

        ImGui::SetNextWindowSize(ImVec2((float) m_editorContext.layout.Log_Width, (float) m_editorContext.layout.Log_Height));

        ImGui::Begin("Bottom", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTabBar("BottomTabs"))
        {
            DrawLogTab();
            //DrawEntityTemplateTab();

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};
