#pragma once
#include "editor/EditorContext.h"
#include "core/utils/Colors.h"
#include <SFML/Graphics/Color.hpp>
#include <imgui.h>

class DebugPanel
{
  private:

    EditorContext& m_editorContext;

    void DrawECSTab()
    {
        if (!ImGui::BeginTabItem("World")) return;

        auto& world = *m_editorContext.world;

        m_editorContext.archetypeDebugger.DrawArchetypeGuiTab(
            world.GetEntityManager(), world.GetArchetypeRegistry(), world.GetCommandBuffer(), m_editorContext.inspector);

        ImGui::EndTabItem();
    }

    void DrawRenderingTab()
    {
        if (!ImGui::BeginTabItem("Rendering")) return;

        if (!m_editorContext.renderSystem)
        {
            ImGui::Text("RenderSystem unavailable.");
            ImGui::EndTabItem();
            return;
        }

        auto& renderSystem = *m_editorContext.renderSystem;

        bool canDrawSprites = renderSystem.GetCanDrawSprites();
        if (ImGui::Checkbox("Draw Sprites", &canDrawSprites)) renderSystem.SetCanDrawSprites(canDrawSprites);

        ImGui::Separator();

        bool canDrawText = renderSystem.GetCanDrawText();
        if (ImGui::Checkbox("Draw Texts", &canDrawText)) renderSystem.SetCanDrawTest(canDrawText);

        ImGui::Separator();

        bool canDrawShapes = renderSystem.GetCanDrawShapes();
        if (ImGui::Checkbox("Draw Shapes", &canDrawShapes)) renderSystem.SetCanDrawShapes(canDrawShapes);

        ImGui::Separator();

        bool canDrawColliders = renderSystem.GetCanDrawColliders();
        if (ImGui::Checkbox("Draw Colliders", &canDrawColliders)) renderSystem.SetCanDrawColliders(canDrawColliders);

        ImGui::EndTabItem();
    }

    void DrawPhysicsTab()
    {
        if (!ImGui::BeginTabItem("Physics")) return;

        // Placeholder for future integration (this day will come)
        ImGui::TextColored(Colors::ConcreteGrey_ImGui, "Physics Debug Tools");

        ImGui::Separator();

        static bool showCollisionPairs = false;
        static bool showBroadphase = false;

        ImGui::Checkbox("Show Collision Pairs", &showCollisionPairs);
        ImGui::Checkbox("Show Broadphase Grid", &showBroadphase);

        ImGui::EndTabItem();
    }

    void DrawEditorTab()
    {
        if (!ImGui::BeginTabItem("Editor")) return;

        float tabWidth = ImGui::GetContentRegionAvail().x;

        ImGui::SeparatorText("Grid");
        ImGui::RadioButton("16", &m_editorContext.editorGrid.GetCellSize(), 16);
        ImGui::SameLine();
        ImGui::RadioButton("32", &m_editorContext.editorGrid.GetCellSize(), 32);
        ImGui::SameLine();
        ImGui::RadioButton("48", &m_editorContext.editorGrid.GetCellSize(), 48);
        ImGui::SameLine();
        ImGui::RadioButton("64", &m_editorContext.editorGrid.GetCellSize(), 64);
        ImGui::SameLine();

        ImGui::SetCursorPosX(tabWidth - 2 * (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize("Snap to Grid").x));
        bool& canSnapToGrid = m_editorContext.editorGrid.GetCanSnapToGrid();
        ImGui::Checkbox("Snap to Grid", &canSnapToGrid);

        ImGui::SameLine();

        ImGui::SetCursorPosX(tabWidth - (ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x + ImGui::CalcTextSize("Show Grid").x));
        bool& canShowGrid = m_editorContext.editorGrid.GetCanShowGrid();
        ImGui::Checkbox("Show Grid", &canShowGrid);

        ImGui::Spacing();
        ImGui::Spacing();

        sf::Color& color = m_editorContext.editorGrid.GetCellColor();
        float col[4] = { color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f };
        ImGui::PushItemWidth(225);
        if (ImGui::ColorEdit4("Color", col))
        {
            color = sf::Color(static_cast<uint8_t>(col[0] * 255),
                static_cast<uint8_t>(col[1] * 255),
                static_cast<uint8_t>(col[2] * 255),
                static_cast<uint8_t>(col[3] * 255));
        }
        ImGui::PopItemWidth();

        ImGui::EndTabItem();
    }

  public:

    DebugPanel(EditorContext& editorContext) : m_editorContext(editorContext) { }

    void Draw()
    {
        if (!m_editorContext.world) return;

        auto& layout = m_editorContext.layout;

        ImGui::SetNextWindowPos(ImVec2((float) layout.Debug_X, (float) layout.Debug_Y));

        ImGui::SetNextWindowSize(ImVec2((float) layout.Debug_Width, (float) layout.Debug_Height));

        ImGui::Begin("Debugger",
            nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_HorizontalScrollbar);

        if (ImGui::BeginTabBar("DebugTabs"))
        {
            DrawECSTab();
            DrawRenderingTab();
            DrawPhysicsTab();
            DrawEditorTab();

            if (!m_editorContext.entityTemplateManager) return;

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};
