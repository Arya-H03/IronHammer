#pragma once
#include "editor/EditorContext.h"
#include <imgui.h>

class ViewportPanel
{
  private:

    EditorContext& m_editorContext;

  public:

    ViewportPanel(EditorContext& editorContext) : m_editorContext(editorContext) { }

    void Draw()
    {
        ImGui::SetNextWindowPos(ImVec2((float) m_editorContext.layout.Viewport_X, (float) m_editorContext.layout.Viewport_Y));

        ImGui::SetNextWindowSize(ImVec2((float) m_editorContext.layout.Viewport_Width, (float) m_editorContext.layout.Viewport_Height));

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        auto size = m_editorContext.viewportTexture.getSize();

        ImGui::Image(m_editorContext.viewportTexture.getTexture(), ImVec2((float) size.x - 20, (float) size.y - 20));

        ImGui::End();
    }
};
