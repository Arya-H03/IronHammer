#pragma once
#include "editor/EditorContext.h"
#include "core/utils/Colors.h"
#include <imgui.h>

class DebugPanel
{
  private:

    EditorContext& m_editorContext;

    void DrawECSTab()
    {
        if (!ImGui::BeginTabItem("ECS")) return;

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

        bool canDrawText = renderSystem.GetCanDrawText();
        if (ImGui::Checkbox("Draw Text", &canDrawText)) renderSystem.SetCanDrawTest(canDrawText);

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

        // Placeholder for future physics debugger
        ImGui::TextColored(Colors::ConcreteGrey_ImGui, "Physics Debug Tools");

        ImGui::Separator();

        static bool showCollisionPairs = false;
        static bool showBroadphase = false;

        ImGui::Checkbox("Show Collision Pairs", &showCollisionPairs);
        ImGui::Checkbox("Show Broadphase Grid", &showBroadphase);

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

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};
