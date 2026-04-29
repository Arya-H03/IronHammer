#include "BroadPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/memory/InlineVector.h"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize) : m_windowSize(windowSize)
{
    m_gridRows = m_windowSize.y / m_cellSize;
    m_gridCols = m_windowSize.x / m_cellSize;

    m_broadPhaseGrid.resize(m_gridRows * m_gridCols);
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_broadPhaseQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
}

bool BroadPhaseCollisionSystem::GetCanDisplayGrid() const
{
    return m_canDisplayGrid;
}

bool BroadPhaseCollisionSystem::GetCanHighlightGrid() const
{
    return m_canHighlightGrid;
}

void BroadPhaseCollisionSystem::SetCanDisplayGrid(World* worldPtr, bool val)
{
    m_canDisplayGrid = val;
}

void BroadPhaseCollisionSystem::SetCanHighlightGrid(World* worldPtr, bool val)
{
    m_canHighlightGrid = val;
}

void BroadPhaseCollisionSystem::FillCellsWithOverlappingEntities(World* worldPtr, std::vector<SolverBody>& solverBodies)
{
    ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");

    for (auto& dataList : m_broadPhaseGrid)
    {
        dataList.clear();
    }

    m_broadPhaseQuery->ForEachWithEntity<CCollider, CTransform, CRigidBody>(
        [&](Entity entity, CCollider& colliderComp, CTransform& transformComp, CRigidBody& rigidBodyComp)
        {
            Vect2<int> topLeftCoord = ((transformComp.position + colliderComp.offset - (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> bottomRightCoord = ((transformComp.position + colliderComp.offset + (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> homeCoord = ((transformComp.position + colliderComp.offset) / m_cellSize).Floor();

            Vect2f center = transformComp.position + colliderComp.offset;
            solverBodies.push_back(
                {entity, center, colliderComp.halfSize, rigidBodyComp.inverseMass, colliderComp.mask, colliderComp.layer, &transformComp});

            for (int j = topLeftCoord.y; j <= bottomRightCoord.y; ++j)
            {
                for (int i = topLeftCoord.x; i <= bottomRightCoord.x; ++i)
                {
                    if (j < 0 || j >= (int)m_gridRows || i < 0 || i >= (int)m_gridCols) continue;
                    uint16_t index = j * m_gridCols + i;
                    m_broadPhaseGrid[index].Push({solverBodies.size() - 1, topLeftCoord});
                }
            }
        });
}

void BroadPhaseCollisionSystem::FindCollisionPairs(const std::vector<SolverBody>& solverBodies)
{
    ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/FilterOutCollisionPairs");

    m_potentialCollisionPairs.clear();

    for (uint16_t currentCellIndex = 0; currentCellIndex < m_broadPhaseGrid.size(); ++currentCellIndex)
    {
        const auto& dataList = m_broadPhaseGrid[currentCellIndex];

        for (size_t i = 0; i < dataList.Size(); ++i)
        {
            const BroadPhaseCellData& cellDataA = dataList.list[i];
            const SolverBody& solverBodyA = solverBodies[cellDataA.solverBodyIndex];
            for (size_t j = i + 1; j < dataList.Size(); ++j)
            {
                const BroadPhaseCellData& cellDataB = dataList.list[j];
                const SolverBody& solverBodyB = solverBodies[cellDataB.solverBodyIndex];

                if (!CanCollidersContact(solverBodyA.collisionMask, solverBodyB.collisionMask, solverBodyA.collisionLayer,
                                         solverBodyB.collisionLayer))
                {
                    continue;
                }

                const uint16_t sharedMinX = std::max(cellDataA.minCell.x, cellDataB.minCell.x);
                const uint16_t sharedMinY = std::max(cellDataA.minCell.y, cellDataB.minCell.y);
                const uint16_t ownerCellIndex = sharedMinY * m_gridCols + sharedMinX;
                if (currentCellIndex != ownerCellIndex) continue;

                const bool aIsLower = solverBodyA.entity.id < solverBodyB.entity.id;

                if (aIsLower)
                {
                    m_potentialCollisionPairs.push_back(PotentialCollisionPair{cellDataA.solverBodyIndex, cellDataB.solverBodyIndex});
                }
                else
                {
                    m_potentialCollisionPairs.push_back(PotentialCollisionPair{cellDataB.solverBodyIndex, cellDataA.solverBodyIndex});
                }
            }
        }
    }
}

bool BroadPhaseCollisionSystem::CanCollidersContact(uint32_t colliderMaskA, uint32_t colliderMaskB, Layer colliderLayerA,
                                                    Layer colliderLayerB)
{
    uint32_t bitA = static_cast<uint32_t>(colliderLayerA);
    uint32_t bitB = static_cast<uint32_t>(colliderLayerB);

    bool aWantsB = (colliderMaskA & bitB) != 0;
    bool bWantsA = (colliderMaskB & bitA) != 0;

    return aWantsB && bWantsA;
}

NarrowPhaseSIMDBatch& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr, std::vector<SolverBody>& solverBodies)
{
    FillCellsWithOverlappingEntities(worldPtr, solverBodies);
    FindCollisionPairs(solverBodies);

    m_narrowPhasebatch.Clear();
    for (const PotentialCollisionPair& pair : m_potentialCollisionPairs)
    {
        const SolverBody& solverBodyA = solverBodies[pair.solverBodyAIndex];
        const SolverBody& solverBodyB = solverBodies[pair.solverBodyBIndex];

        m_narrowPhasebatch.aSolverBodyIndex.push_back(pair.solverBodyAIndex);
        m_narrowPhasebatch.bSolverBodyIndex.push_back(pair.solverBodyBIndex);

        m_narrowPhasebatch.aPositionX.push_back(solverBodyA.position.x);
        m_narrowPhasebatch.aPositionY.push_back(solverBodyA.position.y);
        m_narrowPhasebatch.aColliderHalfSizeX.push_back(solverBodyA.colliderHalfSize.x);
        m_narrowPhasebatch.aColliderHalfSizeY.push_back(solverBodyA.colliderHalfSize.y);

        m_narrowPhasebatch.bPositionX.push_back(solverBodyB.position.x);
        m_narrowPhasebatch.bPositionY.push_back(solverBodyB.position.y);
        m_narrowPhasebatch.bColliderHalfSizeX.push_back(solverBodyB.colliderHalfSize.x);
        m_narrowPhasebatch.bColliderHalfSizeY.push_back(solverBodyB.colliderHalfSize.y);
    }

    return m_narrowPhasebatch;
}
