#pragma once
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cmath>
#include <imgui.h>
#include <string>
#include <vector>
#include <cmath>
#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "editor/EditorContext.h"
#include "editor/Viewport.h"
#include "editor/ViewportGizmo.h"
#include "engine/Engine.h"

class ViewportPanel
{
  private:

    EditorContext& m_editorContext;
    ViewportGismo m_viewportGismo;

    void DrawGismo()
    {
        if (m_editorContext.engineMode == EngineMode::Edit)
        {
            Entity liveEntity = m_editorContext.inspector.GetCurrentInspectorEntity();

            if (!m_editorContext.world->ValidateEntity(liveEntity, false)) return;

            CTransform* transformPtr = m_editorContext.world->TryGetComponent<CTransform>(liveEntity);
            if (!transformPtr) return;

            switch (m_editorContext.viewPortGizmMode)
            {
                case GismoMode::None: break;
                case GismoMode::Position: m_viewportGismo.DrawLiveEntityPositionGizmo(transformPtr); break;
                case GismoMode::Scale: m_viewportGismo.DrawLiveEntityScaleGizmo(transformPtr); break;
            }
        }
    }

  public:

    ViewportPanel(EditorContext& editorContext) : m_editorContext(editorContext) { }

    void Draw()
    {
        ImGui::SetNextWindowPos(ImVec2((float) m_editorContext.layout.Viewport_X, (float) m_editorContext.layout.Viewport_Y));
        ImGui::SetNextWindowSize(ImVec2((float) m_editorContext.layout.Viewport_Width, (float) m_editorContext.layout.Viewport_Height));

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        sf::Vector2u size = m_editorContext.viewportTexture.getSize();

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

        DrawGismo();

        ImGui::End();
    }
};
