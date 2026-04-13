#include "BroadPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Colors.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"
#include "rendering/RenderingComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cstddef>
#include <unordered_set>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize) : m_windowSize(windowSize)
{
    m_gridRows = m_windowSize.y / m_cellSize;
    m_gridCols = m_windowSize.x / m_cellSize;
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_broadPhaseQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider,CRigidBody>>();
    PopulateGrid(worldPtr);
}

void BroadPhaseCollisionSystem::PopulateGrid(World* worldPtr)
{
    m_grid.resize(m_gridRows * m_gridCols);
    m_gridDisplayEntities.resize(m_gridRows * m_gridCols);

    for (size_t j = 0; j < m_gridRows; ++j)
    {
        for (size_t i = 0; i < m_gridCols; ++i)
        {
            size_t index = j * m_gridCols + i;

            Vect2f pos(((i * m_cellSize) + (m_cellSize * 0.5f)), ((j * m_cellSize) + (m_cellSize * 0.5f)));
            Vect2<int> coord(i, j);
            m_grid[index] = Cell(coord, pos);

            m_gridDisplayEntities[index] = worldPtr->CreateEntityWithReturn(
                CTransform(pos, Vect2f(1, 1), 0), CShape(4, Colors::DarkSteel_SFML, sf::Color(255, 255, 255, 100), m_cellRadius, 2),
                CText(std::to_string(coord.x) + ", " + std::to_string(coord.y), sf::Color::White,
                      Vect2f(m_cellRadius / 3, m_cellRadius / 3), 15),
                CNotDrawable());
        }
    }
}

void BroadPhaseCollisionSystem::ClearAllCells()
{
    for (size_t i = 0; i < m_grid.size(); ++i)
    {
        m_grid[i].overlapingEntities.clear();
    }
}

void BroadPhaseCollisionSystem::FillCellsWithOverlappingEntities(World* worldPtr)
{
    m_flatGridData.clear();
    m_broadPhaseQuery->ForEachWithEntity<CCollider, CTransform,CRigidBody>(
        [&](Entity entity, CCollider& colliderComp, CTransform& transformComp, CRigidBody& rigidBodyComp)
        {
            Vect2<int> topLeftCoord = ((transformComp.position + colliderComp.offset - (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> bottomRightCoord = ((transformComp.position + colliderComp.offset + (colliderComp.halfSize)) / m_cellSize).Floor();

            for (size_t j = topLeftCoord.y; j <= bottomRightCoord.y; ++j)
            {
                for (size_t i = topLeftCoord.x; i <= bottomRightCoord.x; ++i)
                {
                    if (j < 0 || j >= m_gridRows || i < 0 || i >= m_gridCols) continue;
                    size_t index = j * m_gridCols + i;
                    m_flatGridData.push_back({index, entity, &transformComp, &colliderComp, &rigidBodyComp});
                }
            }
        });

    if (m_canHighlightGrid)
    {
        for (size_t i = 0; i < m_grid.size(); ++i)
        {
            size_t count = m_grid[i].overlapingEntities.size();
            CShape* shape = worldPtr->TryGetComponent<CShape>(m_gridDisplayEntities[i]);
            if (!shape) continue;
            if (count >= 2)
            {
                shape->fillColor = Colors::RustRed_SFML;
            }
            else if (count == 1)
            {
                shape->fillColor = Colors::HazardYellow_SFML;
            }
            else
            {
                shape->fillColor = Colors::DarkSteel_SFML;
            }
        }
    }
}

void BroadPhaseCollisionSystem::FindCollisionPairs()
{
    std::sort(m_flatGridData.begin(), m_flatGridData.end(),
              [&](BroadPhaseCellData& a, BroadPhaseCellData& b) -> bool { return a.cellIndex < b.cellIndex; });

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
                    m_collisionPairs.push_back({m_flatGridData[a].entity, m_flatGridData[b].entity, m_flatGridData[a].transformPtr,
                                                m_flatGridData[a].colliderPtr, m_flatGridData[a].rigidBodyPtr,
                                                m_flatGridData[b].transformPtr, m_flatGridData[b].colliderPtr,
                                                m_flatGridData[b].rigidBodyPtr});
                }
            }
            left = right;
        }
    }
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

    if (val == true)
    {
        for (auto& entity : m_gridDisplayEntities)
        {
            worldPtr->RemoveFromEntity<CNotDrawable>(entity);
        }
        SetCanHighlightGrid(worldPtr, true);
    }
    else
    {
        for (auto& entity : m_gridDisplayEntities)
        {
            worldPtr->AddToEntity(entity, CNotDrawable{});
        }

        SetCanHighlightGrid(worldPtr, false);
    }
}

void BroadPhaseCollisionSystem::SetCanHighlightGrid(World* worldPtr, bool val)
{
    m_canHighlightGrid = val;
    if (!m_canHighlightGrid)
    {
        for (size_t i = 0; i < m_grid.size(); ++i)
        {
            worldPtr->TryGetComponent<CShape>(m_gridDisplayEntities[i])->fillColor = Colors::DarkSteel_SFML;
        }
    }
}

std::vector<CollisionPair>& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr)
{
    ClearAllCells();
    {
        ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");
        FillCellsWithOverlappingEntities(worldPtr);
    }
    {
        ZoneScopedN("BroadPhaseSystem/FindUniqueCollisionPairs");
        FindCollisionPairs();
    }

    return m_collisionPairs;
}
std::vector<CollisionPairOld>& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystemOld(World* worldPtr)
{
    ClearAllCells();
    {
        ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");
        FillCellsWithOverlappingEntitiesOld(worldPtr);
    }
    {
        ZoneScopedN("BroadPhaseSystem/FindUniqueCollisionPairs");
        FindCollisionPairsOld();
    }

    return m_uniquePotentialPairsVector;
}

void BroadPhaseCollisionSystem::FillCellsWithOverlappingEntitiesOld(World* worldPtr)
{
    m_broadPhaseQuery->ForEachWithEntity<CCollider, CTransform>(
        [&](Entity entity, CCollider& colliderComp, CTransform& transformComp)
        {
            Vect2<int> topLeftCoord = ((transformComp.position + colliderComp.offset - (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> bottomRightCoord = ((transformComp.position + colliderComp.offset + (colliderComp.halfSize)) / m_cellSize).Floor();

            for (size_t j = topLeftCoord.y; j <= bottomRightCoord.y; ++j)
            {
                for (size_t i = topLeftCoord.x; i <= bottomRightCoord.x; ++i)
                {
                    if (j < 0 || j >= m_gridRows || i < 0 || i >= m_gridCols) continue;
                    size_t index = j * m_gridCols + i;
                    m_grid[index].overlapingEntities.push_back(entity);
                }
            }
        });
}
void BroadPhaseCollisionSystem::FindCollisionPairsOld()
{
    m_uniquePotentialPairsSet.clear();
    for (auto& cell : m_grid)
    {
        const size_t count = cell.overlapingEntities.size();
        for (size_t i = 0; i < count; ++i)
        {
            for (size_t j = i + 1; j < count; ++j)
            {
                Entity e1 = cell.overlapingEntities[i];
                Entity e2 = cell.overlapingEntities[j];
                m_uniquePotentialPairsSet.insert(CollisionPairOld(e1, e2));
            }
        }
    }

    m_uniquePotentialPairsVector.clear();
    m_uniquePotentialPairsVector.reserve(m_uniquePotentialPairsVector.size());
    for (auto& pair : m_uniquePotentialPairsSet)
    {
        m_uniquePotentialPairsVector.push_back(pair);
    }
}
