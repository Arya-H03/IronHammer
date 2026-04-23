#pragma once
#include "core/CoreComponents.hpp"
#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "editor/EditorContext.h"
#include "editor/Viewport.h"
#include "editor/ViewportGizmo.h"
#include "engine/Engine.h"
#include "rendering/RenderingComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <csignal>
#include <imgui.h>
#include <string>
#include <vector>

class ViewportPanel
{
  private:
    EditorContext& m_editorContext;
    ViewportGismo m_viewportGismo;

    void DrawGismo()
    {
        if (m_editorContext.engineMode == EngineMode::Edit)
        {
            Entity liveEntity = m_editorContext.entityInspector.GetCurrentInspectorEntity();

            if (!m_editorContext.currentWorld->ValidateEntity(liveEntity, false)) return;

            CTransform* transformPtr = m_editorContext.currentWorld->TryGetComponent<CTransform>(liveEntity);
            if (!transformPtr) return;

            switch (m_editorContext.viewPortGizmMode)
            {
                case GismoMode::None:
                    break;
                case GismoMode::Position:
                    m_viewportGismo.DrawLiveEntityPositionGizmo(transformPtr);
                    break;
                case GismoMode::Scale:
                    m_viewportGismo.DrawLiveEntityScaleGizmo(transformPtr);
                    break;
            }
        }
    }

    void HandleViewportMenuPopup()
    {
        if (ImGui::GetIO().MouseClicked[1] && Viewport::IsMouseInsideViewport())
        {
            ImGui::OpenPopup("Test");
        }

        if (ImGui::BeginPopup("Test"))
        {
            if (ImGui::BeginMenu("Create"))
            {
                // Later with Camera/View implmentation make sure,
                // The entities spawn at the middle of the viewport relative to world
                if (ImGui::MenuItem("Square"))
                {
                    m_editorContext.editorWorld->CreateEntityNoReturn(
                        CTransform{{500, 500}, {1, 1}, 0}, CSprite{"Square", {50, 50}, sf::IntRect{{0, 0}, {256, 256}}, sf::Color::White});
                }

                if (ImGui::MenuItem("Circle"))
                {
                    m_editorContext.editorWorld->CreateEntityNoReturn(
                        CTransform{{500, 500}, {1, 1}, 0}, CSprite{"Circle", {50, 50}, sf::IntRect{{0, 0}, {256, 256}}, sf::Color::White});
                }

                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }

    void HandleViewportImage()
    {
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        sf::Vector2u size = m_editorContext.viewportTexture.getSize();

        float texAspect = (float)size.x / size.y;
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
        ImGui::Image(m_editorContext.viewportTexture.getTexture().getNativeHandle(), drawSize, ImVec2(0, 1), ImVec2(1, 0));
    }

    void HandleViewportDragDrop()
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_TEMPLATE"))
            {
                const char* moldName = static_cast<const char*>(payload->Data);

                std::vector<PendingComponent>& components =
                    m_editorContext.currentWorld->CreateEntityFromMoldObject(*m_editorContext.moldManagerPtr->GetMoldByName(moldName));

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

                    transform->previousPosition = transform->position;
                }
                else
                {
                    LOG_ERROR("Tried to drop an Entity in Viewport with no transform");
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

  public:
    ViewportPanel(EditorContext& editorContext) : m_editorContext(editorContext)
    {
    }

    void Draw()
    {
        ImGui::SetNextWindowPos(ImVec2((float)m_editorContext.layout.Viewport_X, (float)m_editorContext.layout.Viewport_Y));
        ImGui::SetNextWindowSize(ImVec2((float)m_editorContext.layout.Viewport_Width, (float)m_editorContext.layout.Viewport_Height));
        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        HandleViewportImage();
        HandleViewportMenuPopup();
        HandleViewportDragDrop();
        DrawGismo();

        ImGui::End();
    }
};
