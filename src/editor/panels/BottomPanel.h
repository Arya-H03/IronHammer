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
        if (!m_editorContext.entityTemplates) return;

        if (ImGui::BeginTabItem("Entity Templates"))
        {
            for (auto& tpl : m_editorContext.entityTemplates->GetEntityTemplates())
            {
                if (ImGui::Selectable(tpl.name.c_str()))
                {
                    // Future
                }

                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("PREFAB", tpl.name.c_str(), tpl.name.size() + 1);

                    ImGui::Text("%s", tpl.name.c_str());
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
            DrawEntityTemplateTab();

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};
