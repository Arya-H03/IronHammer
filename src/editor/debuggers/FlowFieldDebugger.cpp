#include "editor/debuggers/FlowFieldDebugger.h"

#include "core/utils/Colors.h"
#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "engine/Engine.h"
#include "pathfinding/FlowField.h"
#include "pathfinding/FlowFieldSystem.h"
#include "rendering/RenderingComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <cmath>
#include <string>
#include <vector>

FlowFieldDebugger::FlowFieldDebugger()
{
    m_displayMode = EngineMode::Play;
}

void FlowFieldDebugger::DrawTab(DebugTabContext& context)
{
    if (!m_flowFieldSystemPtr) return;

    // Fix me later: Move to only be called once maybe??????
    // Call the frame after creating the visual cells
    UpdateFlowFieldVisualCells(context.worldPtr);
    /////////////////////////////////////

    if (ImGui::BeginTabItem("Pathfinding"))
    {
        if (ImGui::Checkbox("Toggle FlowField", &m_toggleFlowField))
        {
            if (m_toggleFlowField)
                ShowFlowField(context.worldPtr);
            else
                HideFlowField(context.worldPtr);
        }
        if (ImGui::Checkbox("Toggle FlowFieldCells", &m_toggleFlowFieldCells))
        {
            if (m_toggleFlowFieldCells)
                ShowFlowFieldCells(context.worldPtr);
            else
                HideFlowFieldCells(context.worldPtr);
        }
        if (ImGui::Checkbox("Toggle FlowFieldArrows", &m_toggleFlowFieldArrows))
        {
            if (m_toggleFlowFieldArrows)
                ShowFlowFieldArrows(context.worldPtr);
            else
                HideFlowFieldArrows(context.worldPtr);
        }

        ImGui::EndTabItem();
    };
}
void FlowFieldDebugger::UpdateFlowFieldVisualCells(World* worldPtr)
{
    FlowField& flowField = m_flowFieldSystemPtr->m_flowField;

    std::vector<FlowCell>& flowCells = flowField.GetCells();

    for (size_t i = 0; i < m_flowFieldCellVisuals.size(); ++i)
    {
        Entity cellEntity = m_flowFieldCellVisuals[i];
        CShape* shapePtr = worldPtr->TryGetComponent<CShape>(cellEntity);
        CText* textPtr = worldPtr->TryGetComponent<CText>(cellEntity);

        if (!shapePtr || !textPtr)
        {
            LOG_WARNING("Flow cell visual is missing components.");
            continue;
        }

        if (flowCells[i].baseCost == static_cast<FlowCellCost>(FlowCellCostEnum::UnWalkable))
        {
            shapePtr->fillColor = Colors::RustRed_SFML;
        }
        else if (flowCells[i].baseCost == static_cast<FlowCellCost>(FlowCellCostEnum::Target))
        {
            shapePtr->fillColor = Colors::HazardYellow_SFML;
        }
        else
        {
            shapePtr->fillColor = Colors::DarkSteel_SFML;
        }

        // textPtr->content = std::to_string(flowCells[i].GetTotalCost());
    }
}

void FlowFieldDebugger::ResetDebugger()
{
    m_flowFieldCellVisuals.clear();
    m_flowFieldArrowVisuals.clear();
}

void FlowFieldDebugger::ShowFlowField(World* worldPtr)
{
    if (!m_toggleFlowFieldCells)
    {
        m_toggleFlowFieldCells = true;
        ShowFlowFieldCells(worldPtr);
    }
    if (!m_toggleFlowFieldArrows)
    {
        m_toggleFlowFieldArrows = true;
        ShowFlowFieldArrows(worldPtr);
    }
}

void FlowFieldDebugger::HideFlowField(World* worldPtr)
{
    if (m_toggleFlowFieldCells)
    {
        m_toggleFlowFieldCells = false;
        HideFlowFieldCells(worldPtr);
    }
    if (m_toggleFlowFieldArrows)
    {
        m_toggleFlowFieldArrows = false;
        HideFlowFieldArrows(worldPtr);
    }
}

void FlowFieldDebugger::ShowFlowFieldCells(World* worldPtr)
{
    for (auto& cell : m_flowFieldCellVisuals)
    {
        worldPtr->RemoveFromEntity<CNotDrawable>(cell);
    }
}
void FlowFieldDebugger::HideFlowFieldCells(World* worldPtr)
{
    for (auto& cell : m_flowFieldCellVisuals)
    {
        worldPtr->AddToEntity(cell, CNotDrawable{});
    }
}
void FlowFieldDebugger::ShowFlowFieldArrows(World* worldPtr)
{
    for (auto& cell : m_flowFieldArrowVisuals)
    {
        worldPtr->RemoveFromEntity<CNotDrawable>(cell);
    }
}
void FlowFieldDebugger::HideFlowFieldArrows(World* worldPtr)
{
    for (auto& cell : m_flowFieldArrowVisuals)
    {
        worldPtr->AddToEntity(cell, CNotDrawable{});
    }
}

void FlowFieldDebugger::RegisterFlowField(FlowFieldSystem* flowFieldSystem, World* worldPtr)
{
    m_flowFieldSystemPtr = flowFieldSystem;
    ResetDebugger();
    // CreateVisualGrid(worldPtr);
}

void FlowFieldDebugger::UnRegisterFlowField()
{
    m_flowFieldSystemPtr = nullptr;
}

void FlowFieldDebugger::CreateVisualGrid(World* worldPtr)
{
    FlowField& flowField = m_flowFieldSystemPtr->m_flowField;

    std::vector<FlowCell>& flowCells = flowField.GetCells();

    float cellRadius = std::sqrt((flowField.GetCellSize() * flowField.GetCellSize()) / 2);
    size_t gridCols = flowField.GetGridCols();
    size_t gridRows = flowField.GetGridRows();

    m_flowFieldCellVisuals.reserve(gridCols * gridRows);
    m_flowFieldArrowVisuals.reserve(gridCols * gridRows);

    for (size_t i = 0; i < m_flowFieldCellVisuals.capacity(); ++i)
    {
        const FlowCell& flowCell = flowCells[i];

        m_flowFieldCellVisuals.push_back(
            worldPtr->CreateEntityWithReturn(CTransform{flowCell.worldPosition, Vect2f(1, 1), 0}, CNotDrawable(),
                                             CShape(4, Colors::DarkSteel_SFML, sf::Color(255, 255, 255, 100), cellRadius, 2),
                                             CText(std::to_string(flowCell.baseCost + flowCell.dynamiqueCost), sf::Color::White,
                                                   Vect2f{cellRadius / 3, cellRadius / 3}, 12)));
    }

    for (size_t i = 0; i < m_flowFieldArrowVisuals.capacity(); ++i)
    {
        const FlowCell& flowCell = flowCells[i];

        float rotationInDegrees = 0.f;
        if (flowCell.flowDir.x != 0 || flowCell.flowDir.y != 0)
        {
            rotationInDegrees = std::atan2((float)flowCell.flowDir.y, (float)flowCell.flowDir.x) * (180.f / M_PI);
        }

        m_flowFieldArrowVisuals.push_back(worldPtr->CreateEntityWithReturn(
            CTransform{flowCell.worldPosition, Vect2f(1, 1), rotationInDegrees}, CNotDrawable(),
            CSprite("Arrow", Vect2f{cellRadius, cellRadius}, sf::IntRect{{0, 0}, {512, 512}}, Colors::OxidizedGreen_SFML)));
    }
}
