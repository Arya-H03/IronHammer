#pragma once
#include <imgui.h>
#include "ecs/common/ECSCommon.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "core/utils/Colors.h"
#include "physics/NarrowPhaseCollisionSystem.h"
class CollisionDebugger
{
  private:

    BroadPhaseCollisionSystem& m_broadPhaseCollision;
    NarrowPhaseCollisonSystem& m_narrowPhaseCollision;
    ImVec4 defaultTextColor;
    ImVec4 yellowTextColor;
    ImVec4 rustTextColor;

    void BroadPhaseGui() const
    {
        bool canDisplayGrid = m_broadPhaseCollision.GetCanDisplayGrid();
        if (ImGui::Checkbox("Display Grid", &canDisplayGrid))
        {
            m_broadPhaseCollision.SetCanDisplayGrid(canDisplayGrid);
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

                if (cell.overlapingEntities.size() > 2)
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

        if (ImGui::TreeNode("Potential Collision Pairs"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::OxidizedGreen_ImGui);
            ImGui::Text("Pair Count: %zu", m_broadPhaseCollision.m_uniquePotentialPairs.size());
            ImGui::PopStyleColor();
            ImGui::Separator();

            for (auto& pair : m_broadPhaseCollision.m_uniquePotentialPairs)
            {
                ImGui::Text("Entity %i", pair.e1.id);
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::ColdSteelBlue_ImGui);
                ImGui::SameLine();
                ImGui::Text("&");
                ImGui::PopStyleColor();
                ImGui::SameLine();
                ImGui::Text("Entity %i", pair.e2.id);
                ImGui::Separator();
            }
            ImGui::TreePop();
        }
    }

    void NarrowPhaseGui() const
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::OxidizedGreen_ImGui);
        ImGui::Text("Total Collison Count: %zu", m_narrowPhaseCollision.m_collisionPair.size());
        ImGui::PopStyleColor();
        ImGui::Separator();

        for (auto& pair : m_narrowPhaseCollision.m_collisionPair)
        {
            ImGui::Text("Entity %i", pair.e1.id);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::ColdSteelBlue_ImGui);
            ImGui::SameLine();
            ImGui::Text("&");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("Entity %i", pair.e2.id);
            ImGui::Separator();
        }
    }

  public:

    CollisionDebugger(BroadPhaseCollisionSystem& broadPhaseCollision, NarrowPhaseCollisonSystem& narrowPhaseSystem)
        : m_broadPhaseCollision(broadPhaseCollision), m_narrowPhaseCollision(narrowPhaseSystem)
    {
    }

    void CollisionSystemGui() const
    {
        if (ImGui::CollapsingHeader("Broad Phase Collison", ImGuiTreeNodeFlags_None))
        {
            BroadPhaseGui();
        }
        if (ImGui::CollapsingHeader("Narrow Phase Collison", ImGuiTreeNodeFlags_None))
        {
            NarrowPhaseGui();
        }
    }
};
