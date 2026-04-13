#pragma once

#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/query/Query.hpp"
#include "ecs/system/ISystem.h"
#include "editor/debuggers/FlowFieldDebugger.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "game/GameComponents.hpp"
#include "pathfinding/FlowField.h"
#include "pathfinding/FlowFieldGenerator.h"
#include "physics/PhysicsComponents.hpp"
#include "rendering/RenderingComponents.hpp"

#include <cmath>
#include <queue>

class FlowFieldSystem : ISetupSystem
{
    friend class FlowFieldDebugger;

private:
    std::queue<FlowCell*> m_flowCellQueue;

    FlowFieldGenerator m_flowFieldGenerator;
    FlowField m_flowField;

    Query* m_flowFieldTargetQuery;
    Query* m_flowFieldObstacleQuery;
    Query* m_flowFieldAgentQuery;

    void CheckForTargetTags()
    {
        m_flowFieldTargetQuery->ForEach<CTransform, CSprite>(
            [&](CTransform& transform, CSprite& sprite)
            {
                FlowCell* flowCellPtr = m_flowField.TryGetFlowCellByWorldPos(transform.position);
                if (flowCellPtr)
                {
                    flowCellPtr->baseCost = static_cast<FlowCellCost>(FlowCellCostEnum::Target);
                    m_flowCellQueue.push(flowCellPtr);
                }
                // Vect2f bottomLeft{(transform.position.x - sprite.size.x * 0.5f), (transform.position.y - sprite.size.y * 0.5f)};
                // Vect2f topRight{(transform.position.x + sprite.size.x * 0.5f), (transform.position.y + sprite.size.y * 0.5f)};

                // for (size_t j = std::floor(bottomLeft.y); j < std::ceil(topRight.y); ++j)
                // {
                //     for (size_t i = std::floor(bottomLeft.x); i < std::ceil(topRight.x); ++i)
                //     {
                //     }
                // }
            });
    }
    void CheckForObstacleTags()
    {
        m_flowFieldObstacleQuery->ForEach<CTransform, CSprite>(
            [&](CTransform& transform, CSprite& sprite)
            {
                Vect2f bottomLeft{(transform.position.x - sprite.size.x * 0.5f), (transform.position.y - sprite.size.y * 0.5f)};
                Vect2f topRight{(transform.position.x + sprite.size.x * 0.5f), (transform.position.y + sprite.size.y * 0.5f)};

                for (size_t j = std::floor(bottomLeft.y); j < std::ceil(topRight.y); ++j)
                {
                    for (size_t i = std::floor(bottomLeft.x); i < std::ceil(topRight.x); ++i)
                    {
                        FlowCell* flowCellPtr = m_flowField.TryGetFlowCellByWorldPos(Vect2f(i, j));

                        if (!flowCellPtr) continue;
                        flowCellPtr->baseCost = static_cast<FlowCellCost>(FlowCellCostEnum::UnWalkable);
                    }
                }
            });
    }

    void CheckForFlowFieldTags()
    {
        CheckForObstacleTags();
        CheckForTargetTags();
    }

    void CalculateCellCosts()
    {
        while (!m_flowCellQueue.empty())
        {
            FlowCell* flowCellPtr = m_flowCellQueue.front();
            m_flowCellQueue.pop();

            for (const auto& offset : EightDirections)
            {
                FlowCell* neighborCellPtr = m_flowField.TryGetFlowCellNeighbor(*flowCellPtr, offset);

                if (!neighborCellPtr) continue;
                if (neighborCellPtr->baseCost != static_cast<FlowCellCost>(FlowCellCostEnum::UnVisited)) continue;

                m_flowCellQueue.push(neighborCellPtr);
                neighborCellPtr->baseCost = flowCellPtr->baseCost + 1;
            }
        }
    }

    void CalculateFlowDirections()
    {
        for (auto& cell : m_flowField.GetCells())
        {
            Vect2int flowDir = Vect2int::Zero;
            FlowCellCost cheapestNeighbourCost = static_cast<FlowCellCost>(FlowCellCostEnum::UnWalkable);

            for (const auto& dir : EightDirections)
            {
                FlowCell* neighborCellPtr = m_flowField.TryGetFlowCellNeighbor(cell, dir);
                if (!neighborCellPtr) continue;

                FlowCellCost currentNeighbourCost = neighborCellPtr->GetTotalCost();
                if (currentNeighbourCost < cheapestNeighbourCost)
                {
                    cheapestNeighbourCost = currentNeighbourCost;
                    flowDir = dir;
                }
            }
            cell.flowDir = flowDir;
        }
    }

    void SetupNewFlowField(World* worldPtr)
    {
        CheckForFlowFieldTags();
        CalculateCellCosts();
        CalculateFlowDirections();
    }

    void CreateNewFlowField() { m_flowField = FlowField{Vect2f{0, 0}, 1250, 1250, 50}; }

public:
    FlowFieldSystem() = default;
    ~FlowFieldSystem() { SystemDebuggerHub::Instance().GetFlowFieldDebugger().UnRegisterFlowField(); }

    void SetupSystem(World* worldPtr) override
    {
        m_flowFieldTargetQuery = worldPtr->Query<RequiredComponents<CFlowFieldTarget, CTransform, CSprite>>();
        m_flowFieldObstacleQuery = worldPtr->Query<RequiredComponents<CFlowFieldObstacle, CTransform, CSprite>>();
        m_flowFieldAgentQuery = worldPtr->Query<RequiredComponents<CFlowFieldAgent, CTransform, CRigidBody>>();

        CreateNewFlowField();
        SetupNewFlowField(worldPtr);

        SystemDebuggerHub::Instance().GetFlowFieldDebugger().RegisterFlowField(this, worldPtr);
    }

    void UpdateFlowAgents()
    {
        m_flowFieldAgentQuery->ForEach<CTransform, CRigidBody>(
            [&](CTransform& transform, CRigidBody& rigidBody)
            {
                Vect2f agentPos = transform.position;
                FlowCell* currentFlowCell = m_flowField.TryGetFlowCellByWorldPos(agentPos);

                if (currentFlowCell)
                {
                    rigidBody.velocity.x = currentFlowCell->flowDir.x;
                    rigidBody.velocity.y = currentFlowCell->flowDir.y;
                }
            });
    }
};
