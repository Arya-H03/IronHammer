#pragma once

#include "EditorContext.h"
#include "editor/panels/BottomPanel.h"
#include "editor/panels/DebugPanel.h"
#include "editor/panels/InspectorPanel.h"
#include "editor/panels/MenuBarPanel.h"
#include "editor/panels/ViewportPanel.h"

class EditorGui
{
  private:
    EditorContext& m_editorContext;

    ViewportPanel m_viewportPanel;
    InspectorPanel m_inspectorPanel;
    DebugPanel m_debugPanel;
    BottomPanel m_bottomPanel;
    MenuBarPanel m_menuBarPanel;

  public:
    EditorGui(EditorContext& context)
        : m_editorContext(context), m_viewportPanel(context), m_inspectorPanel(context), m_debugPanel(context), m_bottomPanel(context),
          m_menuBarPanel(context)
    {
    }

    void Update()
    {
        m_menuBarPanel.Draw();
        m_inspectorPanel.Draw();
        m_debugPanel.Draw();
        m_bottomPanel.Draw();
        m_viewportPanel.Draw();
    }

    void ApplyGuiTheme()
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
};
