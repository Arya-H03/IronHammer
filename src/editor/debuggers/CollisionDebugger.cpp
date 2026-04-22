#include "editor/debuggers/CollisionDebugger.h"

#include "core/utils/Colors.h"
#include "imgui.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"
#include "physics/CollisionSystem.h"
#include "physics/NarrowPhaseCollisionSystem.h"

CollisionDebugger::CollisionDebugger()
{
    m_displayMode = EngineMode::Play;
}

void CollisionDebugger::RegisterCollisionSystem(CollisionSystem* collisionSystem)
{
    m_collisionSystemPtr = collisionSystem;
}
void CollisionDebugger::UnRegisterCollsionSystem()
{
    m_collisionSystemPtr = nullptr;
}

void CollisionDebugger::DrawTab(DebugTabContext& context)
{
    if (ImGui::BeginTabItem("Physics"))
    {
        if (ImGui::CollapsingHeader("Broad Phase", ImGuiTreeNodeFlags_None))
        {
            BroadPhaseGui(context.worldPtr);
        }
        if (ImGui::CollapsingHeader("Narrow Phase", ImGuiTreeNodeFlags_None))
        {
            NarrowPhaseGui();
        }
        ImGui::EndTabItem();
    };
}

void CollisionDebugger::BroadPhaseGui(World* worldPtr) const
{
    if (!worldPtr || !m_collisionSystemPtr)
    {
        return;
    }

    BroadPhaseCollisionSystem& broadPhaseCollsionSystem = m_collisionSystemPtr->m_broadPhaseCollisionSystem;

    // bool canDisplayGrid = broadPhaseCollsionSystem.GetCanDisplayGrid();
    // if (ImGui::Checkbox("Display Grid", &canDisplayGrid))
    // {
    //     broadPhaseCollsionSystem.SetCanDisplayGrid(worldPtr, canDisplayGrid);
    // }
    // ImGui::SameLine(0, 50);

    // bool canHighlightgrid = broadPhaseCollsionSystem.GetCanHighlightGrid();
    // if (ImGui::Checkbox("Cell Highlighting", &canHighlightgrid))
    // {
    //     broadPhaseCollsionSystem.SetCanHighlightGrid(worldPtr, canHighlightgrid);
    // }
    // ImGui::Separator();

    // if (ImGui::TreeNode("Cells"))
    // {
    //     ImGui::TreePop();
    // }
    //

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::IndustrialOrange_ImGui);
    ImGui::Text("Potentail Collision Pairs: %zu", m_collisionSystemPtr->m_broadPhaseCollisionSystem.m_potentialCollisionPairs.size());
    ImGui::PopStyleColor();
    ImGui::Separator();

    ImGuiListClipper clipper;
    clipper.Begin((int)broadPhaseCollsionSystem.m_potentialCollisionPairs.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            PotentialCollisionPair& pair = broadPhaseCollsionSystem.m_potentialCollisionPairs[i];
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
    for (const auto& pair : m_collisionSystemPtr->m_broadPhaseCollisionSystem.m_potentialCollisionPairs)
    {
    }
    // if (ImGui::TreeNode("Potential Collision Pairs"))
    // {

    //     ImGui::TreePop();
    // }
}

void CollisionDebugger::NarrowPhaseGui() const
{
    if (!m_collisionSystemPtr)
    {
        return;
    }
    NarrowPhaseCollisionSystem& narrowPhaseCollisionSystem = m_collisionSystemPtr->m_narrowPhaseCollisionSystem;

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::OxidizedGreen_ImGui);
    ImGui::Text("Confirmed Collison Pairs: %zu", narrowPhaseCollisionSystem.m_collisionPenetrationData.size());
    ImGui::PopStyleColor();
    ImGui::Separator();

    ImGuiListClipper clipper;
    clipper.Begin((int)narrowPhaseCollisionSystem.m_collisionPenetrationData.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            CollisionCorrectionData pair = narrowPhaseCollisionSystem.m_collisionPenetrationData[i];
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
