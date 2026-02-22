#pragma once
#include <imgui.h>
#include <utility>
#include "ecs/common/ECSCommon.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "core/utils/Colors.h"
#include "physics/NarrowPhaseCollisionSystem.h"
class CollisionDebugger
{
  private:

    BroadPhaseCollisionSystem& m_broadPhaseCollision;
    NarrowPhaseCollisionSystem& m_narrowPhaseCollision;
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
            ImGuiListClipper clipper;
            clipper.Begin((int) m_broadPhaseCollision.m_grid.size());

            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                {
                    Cell& cell = m_broadPhaseCollision.m_grid[i];
                    ImGui::PushID(&cell);

                    if (cell.overlapingEntities.size() > 2)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, Colors::RustRed_ImGui);
                    }
                    else if (cell.overlapingEntities.size() == 1)
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
            }
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Potential Collision Pairs"))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::OxidizedGreen_ImGui);
            ImGui::Text("Pair Count: %zu", m_broadPhaseCollision.m_uniquePotentialPairsVector.size());
            ImGui::PopStyleColor();
            ImGui::Separator();

            ImGuiListClipper clipper;
            clipper.Begin((int) m_broadPhaseCollision.m_uniquePotentialPairsVector.size());

            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                {
                    const auto& pair = m_broadPhaseCollision.m_uniquePotentialPairsVector[i];

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
            ImGui::TreePop();
        }
    }

    void NarrowPhaseGui() const
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::OxidizedGreen_ImGui);
        ImGui::Text("Total Collison Count: %zu", m_narrowPhaseCollision.m_collisionPair.size());
        ImGui::PopStyleColor();
        ImGui::Separator();

        ImGuiListClipper clipper;
        clipper.Begin((int) m_narrowPhaseCollision.m_collisionPair.size());

        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                CollisionPair pair = m_narrowPhaseCollision.m_collisionPair[i];
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
    }

  public:

    CollisionDebugger(BroadPhaseCollisionSystem& broadPhaseCollision, NarrowPhaseCollisionSystem& narrowPhaseSystem)
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
