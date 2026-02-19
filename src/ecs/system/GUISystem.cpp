#include "ecs/system/GUISystem.h"
#include "Tracy.hpp"
#include "imgui.h"

void GUISystem::HandleGUISystem()
{
    ZoneScoped;
    ImGui::SetNextWindowSize(ImVec2(425, 400), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(5, 5), ImGuiCond_Once);

    ImGui::Begin("Debug");

    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
    {
        if (ImGui::BeginTabItem("ECS"))
        {
            m_archetypeDebugger.DrawArchetypeGuiTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Rendering"))
        {
            bool canDrawText = m_renderSystem.GetCanDrawText();
            if (ImGui::Checkbox("Draw Text", &canDrawText))
            {
                m_renderSystem.SetCanDrawTest(canDrawText);
            }
            ImGui::Separator();

            bool canDrawShapes = m_renderSystem.GetCanDrawShapes();
            if (ImGui::Checkbox("Draw Shapes", &canDrawShapes))
            {
                m_renderSystem.SetCanDrawShapes(canDrawShapes);
            }
            ImGui::Separator();

            bool canDrawColliders = m_renderSystem.GetCanDrawColliders();
            if (ImGui::Checkbox("Draw Colliders", &canDrawColliders))
            {
                m_renderSystem.SetCanDrawColliders(canDrawColliders);
            }


            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Physics"))
        {
            //ImGui::Text("This is the Physics tab!");
            if (ImGui::CollapsingHeader("Broad Phase Collison", ImGuiTreeNodeFlags_None))
            {
                m_collisionDebugger.BroadPhaseGui();
            }
            if (ImGui::CollapsingHeader("Narrow Phase Collison", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("IsItemHovered: %d", ImGui::IsItemHovered());
                for (int i = 0; i < 5; i++) ImGui::Text("Some content %d", i);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    //ImGui::Separator();
    ImGui::End();
}

void GUISystem::AppleGUITheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // ---- Shape & Spacing ----
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 1.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    style.WindowPadding = ImVec2(12, 10);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);

    // ---- Colors ----
    ImVec4* c = style.Colors;

    // Backgrounds
    c[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.10f, 1.00f); // Gunmetal
    c[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.11f, 0.12f, 1.00f);
    c[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.09f, 0.98f);

    // Borders
    c[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.26f, 1.00f);
    c[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Text
    c[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);

    // Headers (TreeNodes, Collapsing)
    c[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.72f, 0.60f, 0.12f, 0.65f); // Hazard yellow
    c[ImGuiCol_HeaderActive] = ImVec4(0.85f, 0.72f, 0.15f, 0.85f);

    // Buttons
    c[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.72f, 0.60f, 0.12f, 0.70f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.85f, 0.72f, 0.15f, 0.95f);

    // Frame BG (inputs, sliders)
    c[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.16f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.72f, 0.60f, 0.12f, 0.45f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.72f, 0.15f, 0.75f);

    // Tabs
    c[ImGuiCol_Tab] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(0.72f, 0.60f, 0.12f, 0.65f);
    c[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.23f, 1.00f);
    c[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
    c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);

    // Title bar
    c[ImGuiCol_TitleBg] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.07f, 0.07f, 0.08f, 1.00f);

    // Scrollbar
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.26f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.72f, 0.60f, 0.12f, 0.60f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.85f, 0.72f, 0.15f, 0.90f);

    // Separators
    c[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.60f, 0.12f, 0.70f);
    c[ImGuiCol_SeparatorActive] = ImVec4(0.85f, 0.72f, 0.15f, 0.90f);
}
