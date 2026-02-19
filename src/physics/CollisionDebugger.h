#pragma once
#include <imgui.h>
#include "ecs/common/ECSCommon.h"
#include "physics/BroadPhaseCollisionSystem.h"
class CollisionDebugger
{
  private:
    BroadPhaseCollisionSystem& m_broadPhaseCollision;

    void BroadPhaseCellGui() const
    {
        for (size_t i = 0; i < m_broadPhaseCollision.m_grid.size(); ++i)
        {
            ImGui::PushID(i);
            Cell& cell = m_broadPhaseCollision.m_grid[i];

            if (cell.overlapingEntities.size() > 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            }

            if (ImGui::TreeNode("", "Cell (%i, %i)", cell.coord.x, cell.coord.y))
            {
                for (size_t j = 0; j < cell.overlapingEntities.size(); ++j)
                {
                    ImGui::Text("Entity %i", cell.overlapingEntities[j].id);
                }

                ImGui::TreePop();
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
            ImGui::Separator();
        }
    }

  public:
    CollisionDebugger(BroadPhaseCollisionSystem& broadPhaseCollision) : m_broadPhaseCollision(broadPhaseCollision) {}

    void BroadPhaseGui() const { BroadPhaseCellGui(); }
};
