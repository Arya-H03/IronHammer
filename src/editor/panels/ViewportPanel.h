#pragma once
#include "assets/AssetManager.h"
#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "editor/EditorContext.h"
#include "editor/Viewport.h"
#include "engine/Engine.h"
#include <cmath>
#include <imgui.h>
#include <string>
#include <vector>
#include <cmath>

class ViewportPanel
{
  private:

    enum class GizmoAxis
    {
        X,
        Y,
        XY
    };

    EditorContext& m_editorContext;
    ImVec2 m_gizmoCenterSize { 16, 16 };
    ImVec2 m_gizmoUpArrowSize { 100, 10 };
    ImVec2 transfromGizmoUpArrowButtonSize { 10, 100 };

    void DrawGizmoButton(const char* id, ImVec2 screenPos, ImTextureID texture, ImVec2 size, Vect2f& dragOffset, Vect2f& position, GizmoAxis axis)
    {
        ImGui::SetCursorScreenPos(screenPos);
        ImGui::ImageButton(id, texture, size);

        if (ImGui::IsItemActivated()) dragOffset = position - Viewport::ScreenToViewportMouse();

        if (ImGui::IsItemActive())
        {
            Vect2f mouse = Viewport::ScreenToViewportMouse();
            if (axis == GizmoAxis::X || axis == GizmoAxis::XY) position.x = mouse.x + dragOffset.x;
            if (axis == GizmoAxis::Y || axis == GizmoAxis::XY) position.y = mouse.y + dragOffset.y;
        }
    }

    void DrawLiveEntityTransformGismo()
    {
        Entity entity = m_editorContext.inspector.GetCurrentInspectorEntity();
        if (entity.id == InvalidEntityID) return;

        CTransform* transform = m_editorContext.world->TryGetComponent<CTransform>(entity);
        if (!transform) return;

        ImVec2 entityScreenPos = Viewport::WorldToViewportGui(transform->position);
        static Vect2f dragOffset;

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

        DrawGizmoButton("CenterGizmo",
            ImVec2(entityScreenPos.x - m_gizmoCenterSize.x * 0.5f, entityScreenPos.y - m_gizmoCenterSize.y * 0.5f),
            AssetManager::Instance().GetTextureID("Square"),
            m_gizmoCenterSize,
            dragOffset,
            transform->position,
            GizmoAxis::XY);

        DrawGizmoButton("RightArrowGizmo",
            ImVec2(entityScreenPos.x + m_gizmoCenterSize.x * 0.5f, entityScreenPos.y - m_gizmoUpArrowSize.y * 0.5f),
            AssetManager::Instance().GetTextureID("RightArrow"),
            m_gizmoUpArrowSize,
            dragOffset,
            transform->position,
            GizmoAxis::X);

        DrawGizmoButton("UpArrowGizmo",
            ImVec2(entityScreenPos.x - transfromGizmoUpArrowButtonSize.x * 0.5f,
                entityScreenPos.y - m_gizmoCenterSize.y * 0.5f - transfromGizmoUpArrowButtonSize.y),
            AssetManager::Instance().GetTextureID("UpArrow"),
            transfromGizmoUpArrowButtonSize,
            dragOffset,
            transform->position,
            GizmoAxis::Y);

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();
    }

  public:

    ViewportPanel(EditorContext& editorContext) : m_editorContext(editorContext) { }

    void Draw()
    {
        ImGui::SetNextWindowPos(ImVec2((float) m_editorContext.layout.Viewport_X, (float) m_editorContext.layout.Viewport_Y));
        ImGui::SetNextWindowSize(ImVec2((float) m_editorContext.layout.Viewport_Width, (float) m_editorContext.layout.Viewport_Height));

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        auto size = m_editorContext.viewportTexture.getSize();

        float texAspect = (float) size.x / size.y;
        float viewportAspect = viewportSize.x / viewportSize.y;

        ImVec2 drawSize;

        if (viewportAspect > texAspect)
        {
            drawSize.y = viewportSize.y;
            drawSize.x = viewportSize.y * texAspect;
        }
        else
        {
            drawSize.x = viewportSize.x;
            drawSize.y = viewportSize.x / texAspect;
        }

        Viewport::UpdateViewportImage(ImGui::GetCursorScreenPos(), drawSize);
        ImGui::Image(m_editorContext.viewportTexture.getTexture().getNativeHandle(), drawSize);

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_TEMPLATE"))
            {
                const char* templateName = static_cast<const char*>(payload->Data);

                std::vector<PendingComponent>& components =
                    m_editorContext.world->CreateEntityFromTemplate(*m_editorContext.entityTemplateManager->GetTemplateByName(templateName));

                CTransform* transform = ComponentRegistry::GetComponentFromPendings<CTransform>(components);
                if (transform)
                {
                    Vect2f mousePos = Viewport::ScreenToViewportMouse();
                    if (m_editorContext.editorGrid.GetCanSnapToGrid())
                    {
                        int cellSize = m_editorContext.editorGrid.GetCellSize();
                        transform->position.x = std::round(mousePos.x / cellSize) * cellSize;
                        transform->position.y = std::round(mousePos.y / cellSize) * cellSize;
                    }
                    else
                    {
                        transform->position = mousePos;
                    }
                }
                else
                {
                    LOG_ERROR("Tried to drop an Entity in Viewport with no transform");
                }
            }

            ImGui::EndDragDropTarget();
        }

        if (m_editorContext.engineMode == EngineMode::Edit) DrawLiveEntityTransformGismo();

        ImGui::End();
    }
};
