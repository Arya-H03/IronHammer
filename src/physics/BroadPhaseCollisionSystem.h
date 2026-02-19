#pragma once
#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstdint>
#include <unordered_set>
#include <vector>
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityManager.hpp"

struct Cell
{
    Vect2<int> coord;
    Vect2f pos;
    std::vector<Entity> overlapingEntities;

    Cell() = default;
    Cell(Vect2<int> c, Vect2f p) : coord(c), pos(p) {}
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
    Vect2<uint16_t> m_windowSize;
    std::vector<Cell> m_grid;

    std::vector<Entity> m_entities;
    std::unordered_set<CollisionPair, PairHash> m_uniquePairs;

    EntityManager& m_entityManger;
    Query& broadPhaseQuery;

    // Flags
    bool m_toggleGrid = true;
    bool m_canHighlightGrid = true;

    void PopulateGrid();
    void ClearAllCells();
    void FillCellsWithOverlappingEntities();
    void FindUniqueCollisionPairs();

  public:
    BroadPhaseCollisionSystem(EntityManager& entityManager,
                              ArchetypeRegistry& archetypeRegistry,
                              Vect2<uint16_t> windowSize);

    void HandleBroadPhaseCollisionSystem();

    bool GetToggleGrid() const;
    bool GetCanHighlightGrid() const;
    void SetToggleGrid(bool val);
    void SetCanHighlightGrid(bool val);
};
