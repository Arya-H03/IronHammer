#pragma once
#include "core/utils/Debug.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "editor/EditorContext.h"
#include "editor/Viewport.h"
#include <imgui.h>
#include <string>
#include <vector>

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

        auto size = m_editorContext.viewportTexture.getSize();
        ImGui::Image(m_editorContext.viewportTexture.getTexture(), ImVec2((float) size.x - 20, (float) size.y - 20));

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
                    transform->position = Viewport::ScreenToViewportMouse();
                }
                else
                {
                    Log_Error("Tried to drop an Entity in Viewport with no transform");
                }
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::End();
    }
};
