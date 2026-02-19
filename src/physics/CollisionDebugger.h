#pragma once
#include <imgui.h>
#include "ecs/common/ECSCommon.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "core/utils/Colors.h"
class CollisionDebugger
{
  private:
    BroadPhaseCollisionSystem& m_broadPhaseCollision;
    ImVec4 defaultTextColor;
    ImVec4 yellowTextColor;
    ImVec4 rustTextColor;

    void BroadPhaseCellGui() const
    {
        bool canDrawText = true;
        if (ImGui::Checkbox("Toggle Grid", &canDrawText))
        {
            // m_renderSystem.SetCanDrawTest(canDrawText);
        }
        ImGui::SameLine(0, 50);

        bool canHighlightgrid = m_broadPhaseCollision.GetCanHighlightGrid();
        if (ImGui::Checkbox("Cell Highlighting", &canHighlightgrid))
        {
            m_broadPhaseCollision.SetCanHighlightGrid(canHighlightgrid);
        }
        ImGui::Separator();

        if (ImGui::TreeNode("Cells"))
        {
            for (size_t i = 0; i < m_broadPhaseCollision.m_grid.size(); ++i)
            {
                ImGui::PushID(i);
                Cell& cell = m_broadPhaseCollision.m_grid[i];

                if(cell.overlapingEntities.size() > 2)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::RustRed_ImGui);
                }
                else if (cell.overlapingEntities.size() > 0 && cell.overlapingEntities.size() < 2)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::HazardYellow_ImGui);
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, Colors::Gunmetal_ImGui);
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
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Unique Pairs"))
        {
            for (auto& pair : m_broadPhaseCollision.m_uniquePairs)
            {
                ImGui::Text("Entity %i & Entity %i", pair.e1.id,pair.e2.id);
                ImGui::Separator();
            }
            ImGui::TreePop();
        }
    }

  public:
    CollisionDebugger(BroadPhaseCollisionSystem& broadPhaseCollision) : m_broadPhaseCollision(broadPhaseCollision) {}

    void BroadPhaseGui() const { BroadPhaseCellGui(); }
};
