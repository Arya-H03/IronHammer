#include "editor/debuggers/CollisionDebugger.h"

#include "core/utils/Colors.h"
#include "imgui.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"
#include "physics/CollisionEventSystem.h"
#include "physics/CollisionSystem.h"
#include "physics/NarrowPhaseCollisionSystem.h"

#include <SFML/Graphics/Color.hpp>

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
        if (ImGui::CollapsingHeader("Collision Events", ImGuiTreeNodeFlags_None))
        {
            CollisionEventsGui();
        }
        ImGui::EndTabItem();
    };
}

void CollisionDebugger::BroadPhaseGui(World* worldPtr) const
{
    if (!worldPtr || !m_collisionSystemPtr) return;

    const SolverBodyPairs& solverBodyPairs = m_collisionSystemPtr->m_broadPhaseCollisionSystem.m_solverBodyPairs;

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::IndustrialOrange_ImGui);
    ImGui::Text("Potential Collision Pairs: %zu", m_collisionSystemPtr->m_broadPhaseCollisionSystem.m_solverBodyPairs.Count());
    ImGui::PopStyleColor();
    ImGui::Separator();

    ImGuiListClipper clipper;
    clipper.Begin((int)solverBodyPairs.Count());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            ImGui::Text("Entity %i", solverBodyPairs.bodyAIndices[i]);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::ColdSteelBlue_ImGui);
            ImGui::SameLine();
            ImGui::Text("&");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("Entity %i", solverBodyPairs.bodyBIndices[i]);
            ImGui::Separator();
        }
    }
}

void CollisionDebugger::NarrowPhaseGui() const
{
    if (!m_collisionSystemPtr) return;

    const CollisionResults& collisionResults = m_collisionSystemPtr->m_narrowPhaseCollisionSystem.m_collisionResults;

    ImGui::PushStyleColor(ImGuiCol_Text, Colors::OxidizedGreen_ImGui);
    ImGui::Text("Confirmed Collison Pairs: %zu", collisionResults.Count());

    ImGui::PopStyleColor();
    ImGui::Separator();

    ImGuiListClipper clipper;
    clipper.Begin((int)collisionResults.Count());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            uint16_t sloverBodyAIndex = collisionResults.solverBodyAIndex[i];
            uint16_t sloverBodyBIndex = collisionResults.solverBodyBIndex[i];
            ImGui::Text("Entity %i", m_collisionSystemPtr->m_solverBodies.entites[sloverBodyAIndex].id);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::ColdSteelBlue_ImGui);
            ImGui::SameLine();
            ImGui::Text("&");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("Entity %i", m_collisionSystemPtr->m_solverBodies.entites[sloverBodyBIndex].id);
            ImGui::Separator();
        }
    }
}

void CollisionDebugger::CollisionEventsGui() const
{
    if (!m_collisionSystemPtr) return;
    CollisionEventSystem& collisionEventSystem = m_collisionSystemPtr->m_collsionEventSystem;

    if (ImGui::TreeNode("Previous Frame Events"))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::HazardYellow_SFML);
        ImGui::Text("Total Count: %zu ", collisionEventSystem.m_previousFramePairs.size());
        ImGui::PopStyleColor();
        ImGui::Separator();

        for (const auto& collisionPair : collisionEventSystem.m_previousFramePairs)
        {
            bool isCollisionPairInCurrent = collisionEventSystem.m_currentFramePairs.contains(collisionPair);
            if (!isCollisionPairInCurrent)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::RustRed_SFML);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::ConcreteGrey_SFML);
            }

            ImGui::Text("Entity %i", collisionPair.e1.id);
            ImGui::SameLine();
            ImGui::Text("&");
            ImGui::SameLine();
            ImGui::Text("Entity %i", collisionPair.e2.id);
            ImGui::PopStyleColor();
            ImGui::Separator();
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Current Frame Events"))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::HazardYellow_SFML);
        ImGui::Text("Total Count: %zu ", collisionEventSystem.m_currentFramePairs.size());
        ImGui::PopStyleColor();
        ImGui::Separator();

        for (const auto& collisionPair : collisionEventSystem.m_currentFramePairs)
        {
            bool isCollisionPairInPrevious = collisionEventSystem.m_previousFramePairs.contains(collisionPair);
            if (!isCollisionPairInPrevious)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::OxidizedGreen_SFML);
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::ConcreteGrey_SFML);
            }
            ImGui::Text("Entity %i", collisionPair.e1.id);
            ImGui::SameLine();
            ImGui::Text("&");
            ImGui::SameLine();
            ImGui::Text("Entity %i", collisionPair.e2.id);
            ImGui::Separator();
            ImGui::PopStyleColor();
        }
        ImGui::TreePop();
    }
}
