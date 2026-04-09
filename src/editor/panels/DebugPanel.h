#pragma once
#include "editor/EditorContext.h"
#include "editor/debuggers/IDebugTab.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "engine/Engine.h"

#include <SFML/Graphics/Color.hpp>
#include <imgui.h>

class DebugPanel
{
  private:
    EditorContext& m_editorContext;

  public:
    DebugPanel(EditorContext& editorContext) : m_editorContext(editorContext) {}

    void Draw()
    {
        if (!m_editorContext.world) return;

        auto& layout = m_editorContext.layout;

        ImGui::SetNextWindowPos(ImVec2((float)layout.Debug_X, (float)layout.Debug_Y));

        ImGui::SetNextWindowSize(ImVec2((float)layout.Debug_Width, (float)layout.Debug_Height));

        ImGui::Begin("Debugger", nullptr,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_HorizontalScrollbar);

        DebugTabContext context{m_editorContext.world, &m_editorContext.entityInspector};

        if (ImGui::BeginTabBar("DebugTabs"))
        {

            for (auto& tab : SystemDebuggerHub::Instance().GetDebugTabs())
            {
                if (tab->GetDisplayMode() == m_editorContext.engineMode || tab->GetDisplayMode() == EngineMode::Both)
                {
                    tab->DrawTab(context);
                }
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    };
};
