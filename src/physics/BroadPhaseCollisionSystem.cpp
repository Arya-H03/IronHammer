#include "BroadPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/memory/InlineVector.h"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "editor/Viewport.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize) : m_windowSize(windowSize)
{
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_gridRows = (Viewport::GetSize().y / m_cellSize) + 5;
    m_gridCols = (Viewport::GetSize().x / m_cellSize) + 5;
    m_broadPhaseGrid.resize(m_gridRows * m_gridCols);

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
            Vect2f center = transformComp.position + colliderComp.offset;
            Vect2<uint8_t> topLeftCell = ((center - (colliderComp.halfSize)) / m_cellSize).FloorCast<uint8_t>();
            Vect2<uint8_t> bottomRightCell = ((center + (colliderComp.halfSize)) / m_cellSize).FloorCast<uint8_t>();

            solverBodies.push_back(
                {entity, center, colliderComp.halfSize, rigidBodyComp.inverseMass, colliderComp.mask, colliderComp.layer, &transformComp});

            for (int j = topLeftCell.y; j <= bottomRightCell.y; ++j)
            {
                for (int i = topLeftCell.x; i <= bottomRightCell.x; ++i)
                {
                    if (j < 0 || j >= (int)m_gridRows || i < 0 || i >= (int)m_gridCols) continue;
                    uint16_t index = j * m_gridCols + i;
                    m_broadPhaseGrid[index].Push({static_cast<uint16_t>(solverBodies.size() - 1), topLeftCell});
                }
            }
        });
}

void BroadPhaseCollisionSystem::FindCollisionPairs(const std::vector<SolverBody>& solverBodies)
{
    ZoneScopedN("BroadPhaseSystem/FindCollisionPairs");

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
                size_t indexA = aIsLower ? cellDataA.solverBodyIndex : cellDataB.solverBodyIndex;
                size_t indexB = aIsLower ? cellDataB.solverBodyIndex : cellDataA.solverBodyIndex;
                const SolverBody& sA = solverBodies[indexA];
                const SolverBody& sB = solverBodies[indexB];

                {
                    m_narrowPhasebatch.aSolverBodyIndex.push_back(indexA);
                    m_narrowPhasebatch.bSolverBodyIndex.push_back(indexB);

                    m_narrowPhasebatch.aPositionX.push_back(sA.position.x);
                    m_narrowPhasebatch.aPositionY.push_back(sA.position.y);
                    m_narrowPhasebatch.aColliderHalfSizeX.push_back(sA.colliderHalfSize.x);
                    m_narrowPhasebatch.aColliderHalfSizeY.push_back(sA.colliderHalfSize.y);

                    m_narrowPhasebatch.bPositionX.push_back(sB.position.x);
                    m_narrowPhasebatch.bPositionY.push_back(sB.position.y);
                    m_narrowPhasebatch.bColliderHalfSizeX.push_back(sB.colliderHalfSize.x);
                    m_narrowPhasebatch.bColliderHalfSizeY.push_back(sB.colliderHalfSize.y);
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
    ZoneScopedN("CollisionBroadPhase");

    FillCellsWithOverlappingEntities(worldPtr, solverBodies);

    m_narrowPhasebatch.Clear();
    m_narrowPhasebatch.Reserve(m_lastFrameBatchCount);
    FindCollisionPairs(solverBodies);
    m_lastFrameBatchCount = m_narrowPhasebatch.aSolverBodyIndex.size();

    return m_narrowPhasebatch;
}
