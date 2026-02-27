#include "EditorGui.h"
#include "Tracy.hpp"
#include "core/utils/Colors.h"
#include "imgui.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <imgui-SFML.h>

EditorGui::EditorGui(
    World* world, EditorConfig::Layout& editorLayout, RenderSystem& renderSystem, sf::RenderTexture& renderTexture, Vect2<uint16_t> windowSize)
    : m_worldPtr(world)
    , m_renderSystem(renderSystem)
    , m_renderTexture(renderTexture)
    , m_entityInspector(m_worldPtr->GetEntityInspector())
    , m_editorLayout(editorLayout)
    , m_archetypeDebugger(m_worldPtr->GetArchetypeDebugger())
{
}

void EditorGui::DrawDebugWindow()
{
    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Debug_X, (float) m_editorLayout.Debug_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Debug_Width, (float) m_editorLayout.Debug_Height));
    ImGui::Begin("Debugger",
        nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar
            | ImGuiWindowFlags_NoTitleBar);
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
            if (ImGui::Checkbox("Draw Text", &canDrawText)) m_renderSystem.SetCanDrawTest(canDrawText);

            ImGui::Separator();

            bool canDrawShapes = m_renderSystem.GetCanDrawShapes();
            if (ImGui::Checkbox("Draw Shapes", &canDrawShapes)) m_renderSystem.SetCanDrawShapes(canDrawShapes);

            ImGui::Separator();

            bool canDrawColliders = m_renderSystem.GetCanDrawColliders();
            if (ImGui::Checkbox("Draw Colliders", &canDrawColliders)) m_renderSystem.SetCanDrawColliders(canDrawColliders);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Physics"))
        {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorGui::DrawInspectorWindow()
{
    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Inspector_X, (float) m_editorLayout.Inspector_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Inspector_Width, (float) m_editorLayout.Inspector_Height));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    m_entityInspector.DrawInspectorGui();
    ImGui::End();
}

void EditorGui::DrawLogWindow()
{
    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Log_X, (float) m_editorLayout.Log_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Log_Width, (float) m_editorLayout.Log_Height));
    ImGui::Begin("Logs", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    m_logWindow.DrawLogsGui();
    ImGui::End();
}

void EditorGui::DrawViewport()
{
    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Viewport_X, (float) m_editorLayout.Viewport_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Viewport_Width , (float) m_editorLayout.Viewport_Height));
    ImGui::Begin(
        "Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);
    sf::Vector2u texSize = m_renderTexture.getSize();
    ImGui::Image(m_renderTexture.getTexture(), ImVec2((float) texSize.x -20, (float) texSize.y - 20));
    ImGui::End();
}

void EditorGui::DrawMenuBar()
{
    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Menu_X, (float) m_editorLayout.Menu_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Menu_Width, (float) m_editorLayout.Menu_Height));
    ImGui::Begin("MenuBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::End();
}

void EditorGui::HandleGUISystem()
{

    DrawMenuBar();
    DrawInspectorWindow();
    DrawDebugWindow();
    DrawLogWindow();
    DrawViewport();
}

void EditorGui::ApplyGuiTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Shape & Layout (Flat Industrial)
    style.WindowRounding = 2.0f;
    style.FrameRounding = 2.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabRounding = 2.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;

    style.WindowPadding = ImVec2(12, 10);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);

    ImVec4* c = style.Colors;

    // BACKGROUNDS (Very Dark Minimal)
    c[ImGuiCol_WindowBg] = Colors::UltraDark_ImGui;
    c[ImGuiCol_ChildBg] = Colors::DeepPanel_ImGui;
    c[ImGuiCol_PopupBg] = Colors::UltraDark_ImGui;

    // BORDERS
    c[ImGuiCol_Border] = Colors::Brass_ImGui;
    c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

    // TEXT
    c[ImGuiCol_Text] = Colors::ConcreteGrey_ImGui;
    c[ImGuiCol_TextDisabled] = Colors::Disabled_ImGui;

    // HEADERS (Dark base, hazard only on interaction)
    c[ImGuiCol_Header] = Colors::Surface_ImGui;
    c[ImGuiCol_HeaderHovered] = Colors::Selection_ImGui; // subtle hazard alpha
    c[ImGuiCol_HeaderActive] = Colors::HazardYellow_ImGui;

    // BUTTONS (Minimal default)
    c[ImGuiCol_Button] = Colors::Surface_ImGui;
    c[ImGuiCol_ButtonHovered] = Colors::ButtonHover_ImGui;
    c[ImGuiCol_ButtonActive] = Colors::ButtonActive_ImGui;

    // INPUTS / SLIDERS
    c[ImGuiCol_FrameBg] = Colors::Surface_ImGui;
    c[ImGuiCol_FrameBgHovered] = Colors::Selection_ImGui;
    c[ImGuiCol_FrameBgActive] = Colors::ButtonHover_ImGui;

    c[ImGuiCol_SliderGrab] = Colors::NeutralHover_ImGui;
    c[ImGuiCol_SliderGrabActive] = Colors::HazardYellow_ImGui;

    c[ImGuiCol_CheckMark] = Colors::RustRed_ImGui;

    // TABS (Flat industrial)
    c[ImGuiCol_Tab] = Colors::DeepPanel_ImGui;
    c[ImGuiCol_TabHovered] = Colors::Selection_ImGui;
    c[ImGuiCol_TabActive] = Colors::Surface_ImGui;
    c[ImGuiCol_TabUnfocused] = Colors::UltraDark_ImGui;
    c[ImGuiCol_TabUnfocusedActive] = Colors::DeepPanel_ImGui;

    // TITLE BAR
    c[ImGuiCol_TitleBg] = Colors::UltraDark_ImGui;
    c[ImGuiCol_TitleBgActive] = Colors::DeepPanel_ImGui;
    c[ImGuiCol_TitleBgCollapsed] = Colors::UltraDark_ImGui;

    // SCROLLBAR
    c[ImGuiCol_ScrollbarBg] = Colors::UltraDark_ImGui;
    c[ImGuiCol_ScrollbarGrab] = Colors::NeutralHover_ImGui;
    c[ImGuiCol_ScrollbarGrabHovered] = Colors::Selection_ImGui;
    c[ImGuiCol_ScrollbarGrabActive] = Colors::HazardYellow_ImGui;

    // SEPARATORS
    c[ImGuiCol_Separator] = Colors::Separator_ImGui;
    c[ImGuiCol_SeparatorHovered] = Colors::Selection_ImGui;
    c[ImGuiCol_SeparatorActive] = Colors::HazardYellow_ImGui;

    // TEXT SELECTION
    c[ImGuiCol_TextSelectedBg] = Colors::Selection_ImGui;
}
