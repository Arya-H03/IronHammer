#include "editor/debuggers/FlowFieldDebugger.h"

#include "core/utils/Colors.h"
#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "engine/Engine.h"
#include "pathfinding/FlowField.h"
#include "pathfinding/FlowFieldSystem.h"
#include "rendering/RenderingComponents.hpp"

#include <cmath>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <string>
#include <vector>

FlowFieldDebugger::FlowFieldDebugger() { m_displayMode = EngineMode::Play; }

void FlowFieldDebugger::DrawTab(DebugTabContext& context)
{
    if (!m_flowFieldSystemPtr) return;

    // Fix me later: Move to only be called once maybe??????
    // Call the frame after creating the visual cells
    UpdateFlowFieldVisualCells(context.worldPtr);
    /////////////////////////////////////

    if (ImGui::BeginTabItem("Pathfinding")) {
        if (ImGui::Checkbox("Toggle FlowField", &m_toggleFlowField)) {
            if (m_toggleFlowField) ShowFlowField(context.worldPtr);
            else HideFlowField(context.worldPtr);
        }

        ImGui::EndTabItem();
    };
}
void FlowFieldDebugger::UpdateFlowFieldVisualCells(World* worldPtr)
{
    FlowField& flowField = m_flowFieldSystemPtr->m_flowField;

    std::vector<FlowCell>& flowCells = flowField.GetCells();

    for (size_t i = 0; i < flowCells.size(); ++i) {
        Entity  cellEntity = m_flowCellVisuals[i];
        CShape* shapePtr   = worldPtr->TryGetComponent<CShape>(cellEntity);
        CText*  textPtr    = worldPtr->TryGetComponent<CText>(cellEntity);

        if (!shapePtr || !textPtr) {
            LOG_WARNING("Flow cell visual is missing components.");
            continue;
        }

        if (flowCells[i].baseCost == static_cast<FlowCellCost>(FlowCellCostEnum::UnWalkable)) {
            shapePtr->fillColor = Colors::RustRed_SFML;
        }
        else if (flowCells[i].baseCost == static_cast<FlowCellCost>(FlowCellCostEnum::Target)) {
            shapePtr->fillColor = Colors::HazardYellow_SFML;
        }
        else {
            shapePtr->fillColor = Colors::DarkSteel_SFML;
        }

        //       textPtr->content = std::to_string(flowCells[i].baseCost + flowCells[i].dynamiqueCost);
        // textPtr->content = std::to_string(flowCells[i].flowDir.x) + ", " + std::to_string(flowCells[i].flowDir.y);
    }
}

void FlowFieldDebugger::ResetDebugger()
{
    m_flowCellVisuals.clear();
    m_flowDirVisuals.clear();
}

void FlowFieldDebugger::ShowFlowField(World* worldPtr)
{
    for (auto& cell : m_flowCellVisuals) { worldPtr->RemoveFromEntity<CNotDrawable>(cell); }
    for (auto& cell : m_flowDirVisuals) { worldPtr->RemoveFromEntity<CNotDrawable>(cell); }
}

void FlowFieldDebugger::HideFlowField(World* worldPtr)
{
    for (auto& cell : m_flowCellVisuals) { worldPtr->AddToEntity(cell, CNotDrawable{}); }
    for (auto& cell : m_flowDirVisuals) { worldPtr->AddToEntity(cell, CNotDrawable{}); }
}

void FlowFieldDebugger::RegisterFlowField(FlowFieldSystem* flowFieldSystem, World* worldPtr)
{
    m_flowFieldSystemPtr = flowFieldSystem;
    CreateVisualGrid(worldPtr);
}

void FlowFieldDebugger::UnRegisterFlowField() { m_flowFieldSystemPtr = nullptr; }

void FlowFieldDebugger::CreateVisualGrid(World* worldPtr)
{
    FlowField& flowField = m_flowFieldSystemPtr->m_flowField;

    std::vector<FlowCell>& flowCells = flowField.GetCells();

    float  cellRadius = std::sqrt((flowField.GetCellSize() * flowField.GetCellSize()) / 2);
    size_t gridCols   = flowField.GetGridCols();
    size_t gridRows   = flowField.GetGridRows();

    m_flowCellVisuals.reserve(gridCols * gridRows);
    m_flowDirVisuals.reserve(gridCols * gridRows);

    for (size_t i = 0; i < m_flowCellVisuals.capacity(); ++i) {
        const FlowCell& flowCell = flowCells[i];

        m_flowCellVisuals.push_back(worldPtr->CreateEntityWithReturn(
            CTransform{flowCell.worldPosition, Vect2f(1, 1), 0},
            CShape(4, Colors::DarkSteel_SFML, sf::Color(255, 255, 255, 100), cellRadius, 2),
            CText(std::to_string(flowCell.baseCost + flowCell.dynamiqueCost), sf::Color::White,
                  Vect2f{cellRadius / 3, cellRadius / 3}, 12)));
    }

    for (size_t i = 0; i < m_flowDirVisuals.capacity(); ++i) {
        const FlowCell& flowCell = flowCells[i];

        float rotationInDegrees = 0.f;
        if (flowCell.flowDir.x != 0 || flowCell.flowDir.y != 0) {
            rotationInDegrees = std::atan2((float)flowCell.flowDir.y, (float)flowCell.flowDir.x) * (180.f / M_PI);
        }

        m_flowDirVisuals.push_back(worldPtr->CreateEntityWithReturn(
            CTransform{flowCell.worldPosition, Vect2f(1, 1), rotationInDegrees},
            CSprite("Arrow", Vect2f{cellRadius, cellRadius}, sf::IntRect{{0, 0}, {512, 512}}, Colors::OxidizedGreen_SFML)));
    }
}
