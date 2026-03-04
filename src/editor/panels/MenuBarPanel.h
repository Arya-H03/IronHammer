#pragma once
#include "editor/EditorContext.h"
#include "core/utils/Colors.h"
#include "core/utils/Debug.h"
#include <imgui.h>
#include <SFML/Graphics/Texture.hpp>

class MenuBarPanel
{
  private:

    EditorContext& m_editorContext;

    const std::string m_playPath = "assets/play-button.png";
    const std::string m_pausePath = "assets/pause-button.png";
    const std::string m_exitPath = "assets/cross-button.png";

    sf::Texture m_playTexture;
    sf::Texture m_pauseTexture;
    sf::Texture m_exitTexture;

    ImTextureID m_playTexID;
    ImTextureID m_pauseTexID;
    ImTextureID m_exitTexID;

    void LoadTexture(sf::Texture& texture, const std::string& path, ImTextureID& texID)
    {
        if (!texture.loadFromFile(path))
        {
            Log_Warning("Failed to load texture: " + path);
            return;
        }

        texID = (ImTextureID) texture.getNativeHandle();
    }

    void DrawPlayButton(float size)
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

        if (m_editorContext.engineMode == EngineMode::Play)
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Gunmetal_ImGui);
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        if (ImGui::ImageButton("PlayBtn", m_playTexID, ImVec2(size, size)))
        {
            m_editorContext.engine.TogglePlayMode();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }

    void DrawPauseButton(float size)
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));

        if (m_editorContext.engineMode == EngineMode::Play && m_editorContext.isPlayModePaused)
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::Gunmetal_ImGui);
        else
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        if (ImGui::ImageButton("PauseBtn", m_pauseTexID, ImVec2(size, size)))
        {
            m_editorContext.engine.PausePlayMode();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }

    void DrawExitButton(float size)
    {
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        if (ImGui::ImageButton("ExitBtn", m_exitTexID, ImVec2(size, size)))
        {
            m_editorContext.engine.GetRenderWindow().close();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
    }

  public:

    MenuBarPanel(EditorContext& editorContext) : m_editorContext(editorContext)
    {
        LoadTexture(m_playTexture, m_playPath, m_playTexID);
        LoadTexture(m_pauseTexture, m_pausePath, m_pauseTexID);
        LoadTexture(m_exitTexture, m_exitPath, m_exitTexID);
    }

    void Draw()
    {
        auto& layout = m_editorContext.layout;

        ImGui::SetNextWindowPos(ImVec2((float) layout.Menu_X, (float) layout.Menu_Y));

        ImGui::SetNextWindowSize(ImVec2((float) layout.Menu_Width, (float) layout.Menu_Height));

        ImGui::Begin(
            "MenuBar", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

        const float buttonSize = 15.0f;
        const float spacing = ImGui::GetStyle().ItemSpacing.x;

        // Center Play & Pause
        float totalWidth = buttonSize * 2.0f + spacing;
        float availWidth = ImGui::GetContentRegionAvail().x;

        ImGui::SetCursorPos(ImVec2((availWidth - totalWidth) * 0.5f, (layout.Menu_Height - buttonSize) * 0.5f));

        DrawPlayButton(buttonSize);
        ImGui::SameLine();
        DrawPauseButton(buttonSize);

        // Right aligned Exit
        float contentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

        float exitX = contentWidth - buttonSize;

        ImGui::SetCursorPos(ImVec2(exitX, (layout.Menu_Height - buttonSize) * 0.5f));

        DrawExitButton(buttonSize);

        ImGui::End();
    }
};
