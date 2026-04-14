#include "BroadPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <cstddef>
#include <cstdint>
#include <vector>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize) : m_windowSize(windowSize)
{
    m_gridRows = m_windowSize.y / m_cellSize;
    m_gridCols = m_windowSize.x / m_cellSize;

    m_flatGridData.reserve(5000);
    m_flatGridBuffer.reserve(5000);
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
    m_flatGridData.clear();

    m_broadPhaseQuery->ForEachWithEntity<CCollider, CTransform, CRigidBody>(
        [&](Entity entity, CCollider& colliderComp, CTransform& transformComp, CRigidBody& rigidBodyComp)
        {
            Vect2<int> topLeftCoord = ((transformComp.position + colliderComp.offset - (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> bottomRightCoord = ((transformComp.position + colliderComp.offset + (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> homeCoord = ((transformComp.position + colliderComp.offset) / m_cellSize).Floor();

            for (size_t j = topLeftCoord.y; j <= bottomRightCoord.y; ++j)
            {
                for (size_t i = topLeftCoord.x; i <= bottomRightCoord.x; ++i)
                {
                    if (j < 0 || j >= m_gridRows || i < 0 || i >= m_gridCols) continue;
                    size_t index = j * m_gridCols + i;
                    bool isHome = (homeCoord.x == i && homeCoord.y == j);
                    m_flatGridData.push_back({index, entity, isHome, &transformComp, &colliderComp, &rigidBodyComp});
                }
            }
        });
}


void BroadPhaseCollisionSystem::CountSort(std::vector<BroadPhaseCellData>& toSort, int exponent)
{
    size_t size = toSort.size();
    size_t counts[10] = {0};

    m_flatGridBuffer.clear();
    m_flatGridBuffer.resize(size);

    // Calculate digit counts
    for (size_t i = 0; i < size; ++i)
    {
        ++counts[(toSort[i].cellIndex / exponent) % 10];
    }

    for (size_t i = 1; i < 10; ++i)
    {
        counts[i] += counts[i - 1];
    }

    for (int i = size - 1; i >= 0; --i)
    {
        m_flatGridBuffer[counts[(toSort[i].cellIndex / exponent) % 10] - 1] = toSort[i];
        --counts[(toSort[i].cellIndex / exponent) % 10];
    }

    for (size_t i = 0; i < size; ++i)
    {
        toSort[i] = m_flatGridBuffer[i];
    }
}

void BroadPhaseCollisionSystem::RadixSortGridData(std::vector<BroadPhaseCellData>& toSort)
{
    if (toSort.size() <= 1) return;
    size_t maxCellIndex = toSort[0].cellIndex;
    for (size_t i = 0; i < toSort.size(); ++i)
    {
        if (toSort[i].cellIndex > maxCellIndex) maxCellIndex = toSort[i].cellIndex;
    }

    for (size_t exponent = 1; maxCellIndex / exponent > 0; exponent *= 10)
    {
        CountSort(toSort, exponent);
    }
}
void BroadPhaseCollisionSystem::FindCollisionPairs()
{
    {
        ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/SortGridData");
        RadixSortGridData(m_flatGridData);
    }
    {
        ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/FilterOutCollisionPairs");
        m_collisionPairs.clear();
        size_t left = 0;
        for (size_t right = 0; right <= m_flatGridData.size(); ++right)
        {
            bool isEnd = right == m_flatGridData.size();
            bool newCell = !isEnd && (m_flatGridData[right].cellIndex != m_flatGridData[left].cellIndex);

            if (isEnd || newCell)
            {
                for (size_t a = left; a < right; ++a)
                {
                    for (size_t b = a + 1; b < right; ++b)
                    {
                        BroadPhaseCellData& cellA = m_flatGridData[a];
                        BroadPhaseCellData& cellB = m_flatGridData[b];

                        if (!CanCollidersContact(cellA.colliderPtr, cellB.colliderPtr)) return;

                        bool aIsHigher = cellA.entity.id > cellB.entity.id;
                        bool highrIsHome = aIsHigher ? cellA.isHome : cellB.isHome;
                        if (!highrIsHome) continue;

                        if (aIsHigher)
                        {
                            m_collisionPairs.push_back({cellB.entity, cellA.entity, cellB.transformPtr, cellB.colliderPtr,
                                                        cellB.rigidBodyPtr, cellA.transformPtr, cellA.colliderPtr, cellA.rigidBodyPtr});
                        }
                        else
                        {
                            m_collisionPairs.push_back({cellA.entity, cellB.entity, cellA.transformPtr, cellA.colliderPtr,
                                                        cellA.rigidBodyPtr, cellB.transformPtr, cellB.colliderPtr, cellB.rigidBodyPtr});
                        }
                    }
                }
                left = right;
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

std::vector<CollisionPair>& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr)
{
    {
        ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");
        FillCellsWithOverlappingEntities(worldPtr);
    }
    {
        ZoneScopedN("BroadPhaseSystem/FindCollisionPairs");
        FindCollisionPairs();
    }

    return m_collisionPairs;
}
