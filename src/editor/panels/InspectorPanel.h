#pragma once
#include "editor/EditorContext.h"
#include <imgui.h>

class InspectorPanel
{
  private:

    EditorContext& m_editorContext;

  public:

    InspectorPanel(EditorContext& editorContext) : m_editorContext(editorContext) { }

    void Draw()
    {
        if (!m_editorContext.world) return;

        ImGui::SetNextWindowPos(ImVec2((float) m_editorContext.layout.Inspector_X, (float) m_editorContext.layout.Inspector_Y));

        ImGui::SetNextWindowSize(ImVec2((float) m_editorContext.layout.Inspector_Width, (float) m_editorContext.layout.Inspector_Height));

        ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

        m_editorContext.inspector.DrawInspectorGui(
            m_editorContext.world->GetEntityManager(), *m_editorContext.entityTemplateManager, *m_editorContext.world, m_editorContext.engineMode);

        ImGui::End();
    }
};
