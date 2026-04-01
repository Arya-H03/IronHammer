#include "editor/debuggers/FlowFieldDebugger.h"

#include "core/utils/Colors.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "engine/Engine.h"
#include "pathfinding/FlowFieldSystem.h"
#include "rendering/RenderingComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <string>
#include <vector>

FlowFieldDebugger::FlowFieldDebugger() { m_displayMode = EngineMode::Play; }

void FlowFieldDebugger::DrawTab(DebugTabContext& context)
{
    if (!m_flowFieldSystemPtr) return;
    if (ImGui::BeginTabItem("Pathfinding")) {
        if (ImGui::Checkbox("Toggle FlowField", &m_toggleFlowField)) {
            if (m_toggleFlowField) ShowFlowField(context.worldPtr);
            else HideFlowField(context.worldPtr);
        }

        ImGui::EndTabItem();
    };
}

void FlowFieldDebugger::ShowFlowField(World* worldPtr)
{
    for (auto& cell : m_visualCells) { worldPtr->RemoveFromEntity<CNotDrawable>(cell); }
}

void FlowFieldDebugger::HideFlowField(World* worldPtr)
{
    for (auto& cell : m_visualCells) { worldPtr->AddToEntity(cell, CNotDrawable{}); }
}

void FlowFieldDebugger::RegisterFlowField(FlowFieldSystem* flowFieldSystem, World* worldPtr)
{
    m_flowFieldSystemPtr = flowFieldSystem;
    CreateVisualGrid(worldPtr);
}

void FlowFieldDebugger::UnRegisterFlowField() { m_flowFieldSystemPtr = nullptr; }

void FlowFieldDebugger::CreateVisualGrid(World* worldPtr)
{
    const FlowField& flowField = m_flowFieldSystemPtr->m_flowField;

    const std::vector<FlowCell>& flowCells = flowField.GetCells();

    float  cellRadius = std::sqrt((flowField.GetCellSize() * flowField.GetCellSize()) / 2);
    size_t gridCols   = flowField.GetGridCols();
    size_t gridRows   = flowField.GetGridRows();

    m_visualCells.reserve(gridCols * gridRows);

    for (size_t i = 0; i < m_visualCells.capacity(); ++i) {
        const FlowCell& flowCell = flowCells[i];

        m_visualCells.push_back(
            worldPtr->CreateEntityWithReturn(CTransform{flowCell.worldPosition, Vect2f(1, 1), 0},
                                             CShape(4, Colors::DarkSteel_SFML, sf::Color(255, 255, 255, 100), cellRadius, 2),
                                             CText(std::to_string(flowCell.baseCost + flowCell.dynamiqueCost),
                                                   sf::Color::White, Vect2f{cellRadius / 3, cellRadius / 3}, 12)));
    }
}
