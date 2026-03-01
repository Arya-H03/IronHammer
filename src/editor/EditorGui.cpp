#include "EditorGui.h"
#include "Tracy.hpp"
#include "core/utils/Colors.h"
#include "core/utils/Debug.h"
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/entity/EntityInspector.hpp"
#include "engine/Engine.h"
#include "imgui.h"
#include "rendering/RenderSystem.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <imgui-SFML.h>

EditorGui::EditorGui(EditorConfig::Layout& editorLayout, EditorGuiCallbacks& callBacks)
    : m_editorLayout(editorLayout), m_editorGuiCallbacks(callBacks)
{
    if (!playButtonTexture.loadFromFile(playButtonPath))
    {
        Log_Warning("Play Button texture didn't load from file. " + playButtonPath);
    }
    else
    {
        playButtonTexID = (ImTextureID) playButtonTexture.getNativeHandle();
    }

    if (!pauseButtonTexture.loadFromFile(pauseButtonPath))
    {
        Log_Warning("Pause Button texture didn't load from file. " + pauseButtonPath);
    }
    else
    {
        pauseButtonTexID = (ImTextureID) pauseButtonTexture.getNativeHandle();
    }

    if (!exitButtonTexture.loadFromFile(exitButtonPath))
    {
        Log_Warning("Exit Button texture didn't load from file. " + exitButtonPath);
    }
    else
    {
        exitButtonTexID = (ImTextureID) exitButtonTexture.getNativeHandle();
    }
}

void EditorGui::DrawDebugWindow(const ArchetypeDebugger& archetypeDebugger, RenderSystem& renderSystem)
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
            archetypeDebugger.DrawArchetypeGuiTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Rendering"))
        {
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

        if (ImGui::BeginTabItem("Physics"))
        {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
    ImGui::End();
}

void EditorGui::DrawInspectorWindow(const EntityInspector& entityInspector)
{
    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Inspector_X, (float) m_editorLayout.Inspector_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Inspector_Width, (float) m_editorLayout.Inspector_Height));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    entityInspector.DrawInspectorGui();
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

void EditorGui::DrawViewport(sf::RenderTexture& renderTexture)
{

    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Viewport_X, (float) m_editorLayout.Viewport_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Viewport_Width, (float) m_editorLayout.Viewport_Height));
    ImGui::Begin("Viewport",
        nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoScrollbar);

    sf::Vector2u texSize = renderTexture.getSize();
    ImGui::Image(renderTexture.getTexture(), ImVec2((float) texSize.x - 20, (float) texSize.y - 20));
    ImGui::End();
}

void EditorGui::DrawMenuBar(EngineMode engineMode, bool isPlayModePaused)
{
    ImGui::SetNextWindowPos(ImVec2((float) m_editorLayout.Menu_X, (float) m_editorLayout.Menu_Y));
    ImGui::SetNextWindowSize(ImVec2((float) m_editorLayout.Menu_Width, (float) m_editorLayout.Menu_Height));

    ImGui::Begin("MenuBar",
        nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoScrollbar);

    const float buttonSize = 15.0f;
    const float spacing = ImGui::GetStyle().ItemSpacing.x;

    // Center Play & Pause

    float totalWidth = buttonSize * 2.0f + spacing;
    float availWidth = ImGui::GetContentRegionAvail().x;

    ImGui::SetCursorPos(ImVec2((availWidth - totalWidth) * 0.5f, (m_editorLayout.Menu_Height - buttonSize) * 0.5f));

    // Play Button
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    if (engineMode == EngineMode::Play)
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Gunmetal_ImGui);
    else
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

    if (ImGui::ImageButton("PlayBtn", playButtonTexID, ImVec2(buttonSize, buttonSize)))
    {
        m_editorGuiCallbacks.PlayCurrentScene();
    }

    ImGui::PopStyleColor(); // Button
    ImGui::PopStyleColor(); // Border

    ImGui::SameLine();

    // Pause Button
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

    if (engineMode == EngineMode::Play && isPlayModePaused)
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::Gunmetal_ImGui);
    else
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

    if (ImGui::ImageButton("PauseBtn", pauseButtonTexID, ImVec2(buttonSize, buttonSize)))
    {
        m_editorGuiCallbacks.PauseCurrentScene();
    }

    ImGui::PopStyleColor(); // Button
    ImGui::PopStyleColor(); // Border

    // Right aligned Exit

    float contentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

    float exitX = contentWidth - ImGui::GetStyle().WindowPadding.x;

    ImGui::SetCursorPos(ImVec2(exitX, (m_editorLayout.Menu_Height - buttonSize) * 0.5f));

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

    if (ImGui::ImageButton("ExitBtn", exitButtonTexID, ImVec2(buttonSize, buttonSize)))
    {
        m_editorGuiCallbacks.ExitEngine();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    ImGui::End();
}

void EditorGui::HandleGUISystem(
    RenderSystem& renderSystem, World* worldPtr, sf::RenderTexture& renderTexture, EngineMode engineMode, bool isPlayModePaused)
{
    DrawMenuBar(engineMode, isPlayModePaused);
    DrawInspectorWindow(worldPtr->GetEntityInspector());
    DrawDebugWindow(worldPtr->GetArchetypeDebugger(), renderSystem);
    DrawLogWindow();
    DrawViewport(renderTexture);
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
