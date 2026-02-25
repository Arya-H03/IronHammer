#include "GuiSystem.h"
#include "Tracy.hpp"
#include "core/utils/Colors.h"
#include "imgui.h"

GuiSystem::GuiSystem(EntityManager& entityManager,
    CommandBuffer& commandBuffer,
    RenderSystem& renderSystem,
    ArchetypeRegistry& archetypeRegistry,
    const CollisionDebugger& collisionDebugger,
    Vect2<uint16_t> windowSize)
    : m_entityManager(entityManager)
    , m_commandBuffer(commandBuffer)
    , m_renderSystem(renderSystem)
    , m_archetypeRegistry(archetypeRegistry)
    , m_windowSize(windowSize)
    , m_entityInspector(m_entityManager)
    , m_collisionDebugger(collisionDebugger)
    , m_archetypeDebugger(entityManager, archetypeRegistry, m_commandBuffer, m_entityInspector)
{
}

void GuiSystem::DrawDebugWindow()
{
    ImGui::SetNextWindowSize(ImVec2(m_debugWindowWidth, m_debugWindowHeight), ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImVec2(m_offsetFromScreenEdge, m_offsetFromScreenEdge), ImGuiCond_Once);
    ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_NoMove);

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
            m_collisionDebugger.CollisionSystemGui();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void GuiSystem::DrawInspectorWindow()
{
    ImGui::SetNextWindowSize(ImVec2(m_inspectorWindowWidth, m_inspectorWindowHeight), ImGuiCond_Once);
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowPos(ImVec2(m_windowSize.x - m_inspectorWindowWidth - m_offsetFromScreenEdge, m_offsetFromScreenEdge));
    m_entityInspector.DrawInspectorGui();
    ImGui::End();
}

void GuiSystem::DrawLogWindow()
{
    ImGui::SetNextWindowSize(ImVec2(m_logWindowWidth, m_logWindowHeight), ImGuiCond_Once);
    ImGui::Begin("Logs", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImVec2 actualSize = ImGui::GetWindowSize();
    ImGui::SetWindowPos(ImVec2(m_offsetFromScreenEdge, m_windowSize.y - actualSize.y - 70));
    m_logWindow.DrawLogsGui();
    ImGui::End();
}

void GuiSystem::HandleGUISystem()
{
    ZoneScoped;

    DrawDebugWindow();
    DrawInspectorWindow();
    DrawLogWindow();
}

void GuiSystem::AppleGUITheme()
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
