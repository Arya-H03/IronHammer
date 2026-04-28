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

    m_flatGridData.resize(m_gridRows * m_gridCols);
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

void BroadPhaseCollisionSystem::FillCellsWithOverlappingEntities(World* worldPtr)
{
    ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");

    for (auto& dataList : m_flatGridData)
    {
        dataList.clear();
    }

    m_broadPhaseQuery->ForEachWithEntity<CCollider, CTransform, CRigidBody>(
        [&](Entity entity, CCollider& colliderComp, CTransform& transformComp, CRigidBody& rigidBodyComp)
        {
            Vect2<int> topLeftCoord = ((transformComp.position + colliderComp.offset - (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> bottomRightCoord = ((transformComp.position + colliderComp.offset + (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> homeCoord = ((transformComp.position + colliderComp.offset) / m_cellSize).Floor();

            for (int j = topLeftCoord.y; j <= bottomRightCoord.y; ++j)
            {
                for (int i = topLeftCoord.x; i <= bottomRightCoord.x; ++i)
                {
                    if (j < 0 || j >= (int)m_gridRows || i < 0 || i >= (int)m_gridCols) continue;
                    uint16_t index = j * m_gridCols + i;
                    bool isHome = (homeCoord.x == i && homeCoord.y == j);
                    m_flatGridData[index].Push(
                        {&transformComp, &colliderComp, &rigidBodyComp, entity, static_cast<Vect2<uint16_t>>(topLeftCoord)});
                }
            }
        });
}

void BroadPhaseCollisionSystem::FindCollisionPairs()
{
    ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/FilterOutCollisionPairs");

    m_potentialCollisionPairs.clear();

    for (uint16_t currentCellIndex = 0; currentCellIndex < m_flatGridData.size(); ++currentCellIndex)
    {
        const auto& dataList = m_flatGridData[currentCellIndex];

        for (size_t i = 0; i < dataList.Size(); ++i)
        {
            const BroadPhaseCellData& cellA = dataList.list[i];
            for (size_t j = i + 1; j < dataList.Size(); ++j)
            {
                const BroadPhaseCellData& cellB = dataList.list[j];

                if (!CanCollidersContact(cellA.colliderPtr, cellB.colliderPtr)) continue;

                const uint16_t sharedMinX = std::max(cellA.minCell.x, cellB.minCell.x);
                const uint16_t sharedMinY = std::max(cellA.minCell.y, cellB.minCell.y);
                const uint16_t ownerCellIndex = sharedMinY * m_gridCols + sharedMinX;
                if (currentCellIndex != ownerCellIndex) continue;

                const bool aIsLower = cellA.entity.id < cellB.entity.id;

                if (aIsLower)
                {
                    m_potentialCollisionPairs.push_back({cellA.entity, cellB.entity, cellA.transformPtr, cellA.colliderPtr,
                                                         cellA.rigidBodyPtr, cellB.transformPtr, cellB.colliderPtr, cellB.rigidBodyPtr});
                }
                else
                {
                    m_potentialCollisionPairs.push_back({cellB.entity, cellA.entity, cellB.transformPtr, cellB.colliderPtr,
                                                         cellB.rigidBodyPtr, cellA.transformPtr, cellA.colliderPtr, cellA.rigidBodyPtr});
                }
            }
        }
    }
}

bool BroadPhaseCollisionSystem::CanCollidersContact(CCollider* collider1, CCollider* collider2)
{
    uint32_t e1Bit = static_cast<uint32_t>(collider1->layer);
    uint32_t e2Bit = static_cast<uint32_t>(collider2->layer);

    bool e1WantsE2 = (collider1->mask & e2Bit) != 0;
    bool e2WantsE1 = (collider2->mask & e1Bit) != 0;

    if (!e1WantsE2 || !e2WantsE1)
    {
        return false;
    }
    else
    {
        return true;
    }
}

std::vector<PotentialCollisionPair>& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr)
{
    FillCellsWithOverlappingEntities(worldPtr);
    FindCollisionPairs();
    return m_potentialCollisionPairs;
}
