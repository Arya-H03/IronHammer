#pragma once
#include "editor/EditorContext.h"
#include "gui/LogWindow.hpp"
#include "imgui-SFML.h"
#include <SFML/Graphics/Texture.hpp>
#include <imgui.h>

class BottomPanel
{
  private:

    LogWindow m_logWindow;

    float iconSize = 64.f;
    float itemWidth = 88.f;

    EditorContext& m_editorContext;

    void DrawLogTab()
    {
        if (ImGui::BeginTabItem("Logs"))
        {
            m_logWindow.DrawLogsGui();
            ImGui::EndTabItem();
        }
    }

    void DrawEntityTemplateTab()
    {
        if (!m_editorContext.entityTemplateManager) return;

        if (ImGui::BeginTabItem("Entity Templates"))
        {
            float panelWidth = ImGui::GetContentRegionAvail().x;

            int itemsPerRow = std::max(1, (int) (panelWidth / itemWidth));
            int i = 0;

            for (auto& [name, entityTemplate] : m_editorContext.entityTemplateManager->GetEntityTemplates())
            {
                ImGui::BeginGroup();

                float cursorX = ImGui::GetCursorPosX();
                ImGui::SetCursorPosX(cursorX + (itemWidth - iconSize) * 0.5f);

                if (ImGui::ImageButton(name.c_str(), (ImTextureID) entityTemplate->entityTexture.getNativeHandle(), ImVec2(iconSize, iconSize)))
                {
                    m_editorContext.inspector.InspectEntityTemplate(*entityTemplate);
                }

                static bool dragging = false;

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip))
                {
                    ImGui::SetDragDropPayload("ENTITY_TEMPLATE", name.c_str(), name.size() + 1);
                    dragging = true;
                    ImGui::EndDragDropSource();
                }
                else
                    dragging = false;

                if (dragging)
                {
                    ImDrawList* drawList = ImGui::GetForegroundDrawList();
                    ImVec2 mouse = ImGui::GetIO().MousePos;

                    sf::Vector2u texSize = entityTemplate->entityTexture.getSize();
                    ImVec2 halfSize(texSize.x * 0.5f, texSize.y * 0.5f);

                    drawList->AddImage((ImTextureID) entityTemplate->entityTexture.getNativeHandle(),
                        ImVec2(mouse.x - halfSize.x, mouse.y - halfSize.y),
                        ImVec2(mouse.x + halfSize.x, mouse.y + halfSize.y));
                }

                float textWidth = ImGui::CalcTextSize(name.c_str()).x;
                ImGui::SetCursorPosX(cursorX + (itemWidth - textWidth) * 0.6f);
                ImGui::Text("%s", name.c_str());

                ImGui::EndGroup();

                // horizontal layout
                if (++i % itemsPerRow != 0) ImGui::SameLine();
            }

            ImGui::EndTabItem();
        }
    }

  public:

    BottomPanel(EditorContext& editorContext) : m_editorContext(editorContext) { }

    void Draw()
    {
        ImGui::SetNextWindowPos(ImVec2((float) m_editorContext.layout.Log_X, (float) m_editorContext.layout.Log_Y));

        ImGui::SetNextWindowSize(ImVec2((float) m_editorContext.layout.Log_Width, (float) m_editorContext.layout.Log_Height));

        ImGui::Begin("Bottom", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTabBar("BottomTabs"))
        {
            DrawLogTab();
            DrawEntityTemplateTab();

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};
