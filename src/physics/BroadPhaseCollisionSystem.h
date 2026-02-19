#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/Components.hpp"
#include "ecs/entity/EntityManager.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Angle.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <sys/types.h>
#include <unordered_set>
#include <utility>
#include "Tracy.hpp"
#include <vector>

struct Cell
{
    Vect2<int> coord;
    Vect2f pos;
    std::vector<Entity> overlapingEntities;
};

struct CollisionPair
{
    Entity e1, e2;

    bool operator==(const CollisionPair& otherPair) const { return e1 == otherPair.e1 && e2 == otherPair.e2; }
};

struct PairHash
{
    size_t operator()(const CollisionPair& pair) const
    {
        return std::hash<uint32_t>()(pair.e1.id) ^ (std::hash<uint32_t>()(pair.e2.id) << 1);
    }
};

class BroadPhaseCollisionSystem
{
    friend class CollisionDebugger;

  private:
    const float m_cellSize = 48;
    const float m_cellRadius = std::sqrt((m_cellSize * m_cellSize) / 2);
    uint16_t m_cellPerRow, m_cellPerCol;
    std::vector<Cell> m_grid;
    std::vector<Entity> m_entities;
    std::unordered_set<CollisionPair, PairHash> m_uniquePairs;
    Vect2<uint16_t> m_windowSize;
    EntityManager& m_entityManger;

    Query& broadPhaseQuery;

    void PopulateGrid()
    {
        m_grid.resize(m_cellPerCol * m_cellPerRow);
        m_entities.resize(m_cellPerCol * m_cellPerRow);

        for (size_t i = 0; i < m_cellPerCol; ++i)
        {
            for (size_t j = 0; j < m_cellPerRow; ++j)
            {
                size_t index = i * m_cellPerRow + j;

                Vect2<int> coord(i, j);
                Vect2f pos(((j * m_cellSize) + (m_cellSize / 2)), ((i * m_cellSize) + (m_cellSize / 2)));
                m_grid[index] = Cell(coord, pos);

                m_entities[index] =
                    m_entityManger.CreateEntity(CTransform(pos, 45, Vect2f(1, 1)),
                                                CText(std::to_string(coord.x) + ", " + std::to_string(coord.y),
                                                      sf::Color::White,
                                                      Vect2f(m_cellRadius / 3, m_cellRadius / 3),
                                                      18),
                                                CShape(m_cellRadius, 4, sf::Color(1, 1, 1, 200), sf::Color::White, 2));
            }
        }
    }

    void ClearAllCells()
    {
        for (size_t i = 0; i < m_grid.size(); ++i)
        {
            m_grid[i].overlapingEntities.clear();
            m_entityManger.GetComponentRef<CShape>(m_entities[i]).fillColor = sf::Color(1, 1, 1, 200);
        }
    }

    void FillCellsWithOverlappingEntities()
    {
        for (auto& archetype : broadPhaseQuery.matchingArchetypes)
        {
            for (auto& chunk : archetype->GetChunks())
            {
                auto colliderCompRow = chunk.GetComponentRow<CCollider>();
                auto transformCompRow = chunk.GetComponentRow<CTransform>();
                for (size_t i = 0; i < chunk.size; ++i)
                {
                    CCollider& colliderComp = colliderCompRow[i];
                    CTransform& transformComp = transformCompRow[i];

                    // AABB Top-left
                    Vect2<int> topLeftCoord =
                        ((transformComp.position + colliderComp.offset - (colliderComp.size / 2)) / m_cellSize).Floor();

                    // AABB Bottom-right
                    Vect2<int> bottomRightCoord =
                        ((transformComp.position + colliderComp.offset + (colliderComp.size / 2)) / m_cellSize).Ceil();

                    for (size_t r = topLeftCoord.y; r < bottomRightCoord.y; ++r)
                    {
                        for (size_t c = topLeftCoord.x; c < bottomRightCoord.x; ++c)
                        {
                            size_t index = r * m_cellPerRow + c;
                            if (r < 0 || r >= m_cellPerCol || c < 0 || c >= m_cellPerRow) continue;
                            m_grid[index].overlapingEntities.push_back(chunk.entities[i]);
                            m_entityManger.GetComponentRef<CShape>(m_entities[index]).fillColor = sf::Color(255, 204, 0);
                        }
                    }
                }
            }
        }
    }

    void FindUniqueCollisionPairs()
    {
        m_uniquePairs.clear();

        for (auto& cell : m_grid)
        {
            const size_t count = cell.overlapingEntities.size();
            for (size_t i = 0; i < count; ++i)
            {
                for (size_t j = i + 1; j < count; ++j)
                {
                    Entity e1 = cell.overlapingEntities[i];
                    Entity e2 = cell.overlapingEntities[j];

                    if (e1.id > e2.id)
                    {
                        std::swap(e1, e2);
                    }

                    m_uniquePairs.insert({e1, e2});
                }
            }
        }
    }

  public:
    BroadPhaseCollisionSystem(EntityManager& entityManager, ArchetypeRegistry& archetypeRegistry, Vect2<uint16_t> windowSize)
        : m_entityManger(entityManager),
          m_windowSize(windowSize),
          broadPhaseQuery(archetypeRegistry.CreateQuery<CCollider, CTransform>())
    {

        m_cellPerCol = m_windowSize.y / m_cellSize;
        m_cellPerRow = m_windowSize.x / m_cellSize;

        PopulateGrid();
    }

    void HandleBroadPhaseCollisionSystem()
    {
        {
            ZoneScopedN("BroadPhaseSystem/ClearAllCells");
            ClearAllCells();
        }

        {
            ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");
            FillCellsWithOverlappingEntities();
        }

        {
            ZoneScopedN("BroadPhaseSystem/FindUniqueCollisionPairs");
            FindUniqueCollisionPairs();
        }
    }
};
