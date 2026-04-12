#pragma once

#include "assets/AssetManager.h"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "editor/Gizmo.h"
#include "editor/Viewport.h"

#include <cmath>
#include <imgui.h>

class ViewportGismo
{
  private:
    ImVec2 m_gizmoCenterSize{16, 16};
    ImVec2 m_gizmoUpArrowSize{10, 100};
    ImVec2 m_gizmoRightArrowSize{100, 10};

    void DrawGizmoPositionButton(const char* id, ImVec2 screenPos, ImTextureID texture, ImVec2 size, Vect2f& position, GizmoAxis axis)
    {
        ImGui::SetCursorScreenPos(screenPos);
        ImGui::ImageButton(id, texture, size);

        static Vect2f dragOffset;
        if (ImGui::IsItemActivated()) dragOffset = position - Viewport::ScreenToViewportMouse();

        if (ImGui::IsItemActive())
        {
            Vect2f mouse = Viewport::ScreenToViewportMouse();
            if (axis == GizmoAxis::X || axis == GizmoAxis::XY) position.x = mouse.x + dragOffset.x;
            if (axis == GizmoAxis::Y || axis == GizmoAxis::XY) position.y = mouse.y + dragOffset.y;
        }
    }
    void DrawGizmoScaleButton(const char* id, ImVec2 screenPos, ImTextureID texture, ImVec2 size, Vect2f& position, Vect2f& scale,
                              GizmoAxis axis)
    {
        ImGui::SetCursorScreenPos(screenPos);
        ImGui::ImageButton(id, texture, size);

        static Vect2f mouseDragOffest;
        static Vect2f scaleDragOffeset;
        if (ImGui::IsItemActivated())
        {
            mouseDragOffest = Viewport::ScreenToViewportMouse();
            scaleDragOffeset = scale;
        }

        if (ImGui::IsItemActive())
        {
            Vect2f mouse = Viewport::ScreenToViewportMouse();
            Vect2f delta = mouse - mouseDragOffest;
            if (axis == GizmoAxis::X || axis == GizmoAxis::XY) scale.x = scaleDragOffeset.x + delta.x * 0.04;
            if (axis == GizmoAxis::Y || axis == GizmoAxis::XY) scale.y = scaleDragOffeset.y + delta.y * 0.04 * -1;
            scale.x = std::max(scale.x, 0.01f);
            scale.y = std::max(scale.y, 0.01f);
        }
    }

  public:
    void DrawLiveEntityPositionGizmo(CTransform* transformPtr)
    {

        ImVec2 entityScreenPos = Viewport::WorldToViewportGui(transformPtr->position);

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        DrawGizmoPositionButton("CenterGizmo",
                                ImVec2(entityScreenPos.x - m_gizmoCenterSize.x * 0.5f, entityScreenPos.y - m_gizmoCenterSize.y * 0.5f),
                                AssetManager::Instance().GetTextureID("Square"), m_gizmoCenterSize, transformPtr->position, GizmoAxis::XY);

        DrawGizmoPositionButton(
            "RightArrowGizmo", ImVec2(entityScreenPos.x + m_gizmoCenterSize.x * 0.5f, entityScreenPos.y - m_gizmoRightArrowSize.y * 0.5f),
            AssetManager::Instance().GetTextureID("RightArrowSharp"), m_gizmoRightArrowSize, transformPtr->position, GizmoAxis::X);

        DrawGizmoPositionButton(
            "UpArrowGizmo",
            ImVec2(entityScreenPos.x - m_gizmoUpArrowSize.x * 0.5f, entityScreenPos.y - m_gizmoCenterSize.y * 0.5f - m_gizmoUpArrowSize.y),
            AssetManager::Instance().GetTextureID("UpArrowSharp"), m_gizmoUpArrowSize, transformPtr->position, GizmoAxis::Y);

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
    }
    void DrawLiveEntityScaleGizmo(CTransform* transformPtr)
    {
        ImVec2 entityScreenPos = Viewport::WorldToViewportGui(transformPtr->position);

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        ImGui::SetCursorScreenPos(ImVec2(entityScreenPos.x - m_gizmoCenterSize.x * 0.5f, entityScreenPos.y - m_gizmoCenterSize.y * 0.5f));
        ImGui::ImageButton("CenterGizmo", AssetManager::Instance().GetTextureID("Square"), m_gizmoCenterSize);

        DrawGizmoScaleButton("RightArrowGizmo",
                             ImVec2(entityScreenPos.x + m_gizmoCenterSize.x * 0.5f, entityScreenPos.y - m_gizmoRightArrowSize.y * 0.5f),
                             AssetManager::Instance().GetTextureID("RightArrowFlat"), m_gizmoRightArrowSize, transformPtr->position,
                             transformPtr->scale, GizmoAxis::X);

        DrawGizmoScaleButton(
            "UpArrowGizmo",
            ImVec2(entityScreenPos.x - m_gizmoUpArrowSize.x * 0.5f, entityScreenPos.y - m_gizmoCenterSize.y * 0.5f - m_gizmoUpArrowSize.y),
            AssetManager::Instance().GetTextureID("UpArrowFlat"), m_gizmoUpArrowSize, transformPtr->position, transformPtr->scale,
            GizmoAxis::Y);

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
    }
};
