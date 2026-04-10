#pragma once
#include "core/utils/Debug.h"
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

    void DrawMoldTab()
    {
        if (!m_editorContext.moldManagerPtr) return;

        if (ImGui::BeginTabItem("Molds"))
        {
            float panelWidth = ImGui::GetContentRegionAvail().x;

            int itemsPerRow = std::max(1, (int)(panelWidth / itemWidth));
            int i = 0;

            for (auto& [name, mold] : m_editorContext.moldManagerPtr->GetMoldMap())
            {
                ImGui::BeginGroup();

                float cursorX = ImGui::GetCursorPosX();
                ImGui::SetCursorPosX(cursorX + (itemWidth - iconSize) * 0.5f);
                if (ImGui::ImageButton(name.c_str(), (ImTextureID)mold->iconTexture.getNativeHandle(),
                                       ImVec2(iconSize, iconSize)))
                {
                    m_editorContext.entityInspector.InspectMold(*mold);
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
                    DrawMoldPreview(*mold);
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

    void DrawMoldPreview(Mold& entityTemplate)
    {

        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        ImVec2 mouse = ImGui::GetIO().MousePos;

        PendingComponent pendingCSprite = ComponentRegistry::GetPendingComponentFromEntityJson<CSprite>(entityTemplate.moldJson);
        CSprite* spriteComp = ComponentRegistry::GetComponentFromPending<CSprite>(pendingCSprite);
        PendingComponent transformPending = ComponentRegistry::GetPendingComponentFromEntityJson<CTransform>(entityTemplate.moldJson);
        CTransform* transform = ComponentRegistry::GetComponentFromPending<CTransform>(transformPending);

        float previewWidth = iconSize;
        float previewHeight = iconSize;
        float rotationDegree = 0.f;

        if (transform && spriteComp)
        {
            const sf::Texture* texture = spriteComp->texturePtr;
            if (!texture)
            {
                LOG_WARNING("Tired to preview an EntityTemplate with CSprite but no texture.");
                return;
            }

            previewWidth = spriteComp->size.x * transform->scale.x;
            previewHeight = spriteComp->size.y * transform->scale.y;
            float halfWidth = previewWidth * 0.5f;
            float halfHeight = previewHeight * 0.5f;

            float rad = -transform->rotation * (M_PI / 180.f);
            float cosRad = std::cos(rad);
            float sinRad = std::sin(rad);

            auto Rotate = [&](ImVec2 point)
            { return ImVec2(mouse.x + point.x * cosRad - point.y * sinRad, mouse.y + point.x * sinRad + point.y * cosRad); };

            ImVec2 topLeft(-halfWidth, -halfHeight);
            ImVec2 topRight(halfWidth, -halfHeight);
            ImVec2 bottomRight(halfWidth, halfHeight);
            ImVec2 bottomLeft(-halfWidth, halfHeight);

            float u1 = (float)spriteComp->textureRect.position.x / texture->getSize().x;
            float v1 = (float)spriteComp->textureRect.position.y / texture->getSize().y;
            float u2 = (float)(spriteComp->textureRect.position.x + spriteComp->textureRect.size.x) / texture->getSize().x;
            float v2 = (float)(spriteComp->textureRect.position.y + spriteComp->textureRect.size.y) / texture->getSize().y;

            drawList->AddImageQuad((ImTextureID)texture->getNativeHandle(), Rotate(topLeft), Rotate(topRight), Rotate(bottomRight),
                                   Rotate(bottomLeft), ImVec2(u1, v1), ImVec2(u2, v1), ImVec2(u2, u2), ImVec2(u1, v2),
                                   IM_COL32(spriteComp->color.r, spriteComp->color.g, spriteComp->color.b, spriteComp->color.a));
        }
    }

  public:
    BottomPanel(EditorContext& editorContext) : m_editorContext(editorContext)
    {
    }

    void Draw()
    {
        ImGui::SetNextWindowPos(ImVec2((float)m_editorContext.layout.Log_X, (float)m_editorContext.layout.Log_Y));

        ImGui::SetNextWindowSize(ImVec2((float)m_editorContext.layout.Log_Width, (float)m_editorContext.layout.Log_Height));

        ImGui::Begin("Bottom", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTabBar("BottomTabs"))
        {
            DrawLogTab();
            DrawMoldTab();

            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};
