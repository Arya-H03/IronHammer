#pragma once
#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "editor/EditorContext.h"
#include "editor/Viewport.h"
#include <cmath>
#include <imgui.h>
#include <string>
#include <vector>
#include <cmath>

class ViewportPanel
{
  private:

    EditorContext& m_editorContext;

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

        ImGui::End();
    }
};
