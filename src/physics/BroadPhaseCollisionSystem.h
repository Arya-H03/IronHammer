#pragma once
#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstdint>
#include <unordered_set>
#include <vector>
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"

struct Cell
{
    Vect2<int> coord;
    Vect2f pos;
    std::vector<Entity> overlapingEntities;

    Cell() = default;
    Cell(Vect2<int> c, Vect2f p) : coord(c), pos(p) { }
};

struct PotentialCollisionPair
{
    Entity e1, e2;

    bool operator==(const PotentialCollisionPair& otherPair) const { return e1 == otherPair.e1 && e2 == otherPair.e2; }
};

struct PotentialPairHash
{
    size_t operator()(const PotentialCollisionPair& pair) const
    {
        return std::hash<uint32_t>()(pair.e1.id) ^ (std::hash<uint32_t>()(pair.e2.id) << 1);
    }
};

class BroadPhaseCollisionSystem
{
    friend class CollisionDebugger;

  private:

    const float m_cellSize = 8;
    const float m_cellRadius = std::sqrt((m_cellSize * m_cellSize) / 2);
    uint16_t m_cellPerRow, m_cellPerCol;

    Vect2<uint16_t> m_windowSize;
    std::vector<Cell> m_grid;
    std::vector<Entity> m_gridDisplayEntities;

    std::unordered_set<PotentialCollisionPair, PotentialPairHash> m_uniquePotentialPairsSet;
    std::vector<PotentialCollisionPair> m_uniquePotentialPairsVector;

    World* m_worldPtr;
    Query& broadPhaseQuery;

    // Flags
    bool m_canDisplayGrid = false;
    bool m_canHighlightGrid = false;

    void PopulateGrid();
    void ClearAllCells();
    void FillCellsWithOverlappingEntities();
    void FindUniqueCollisionPairs();

  public:

    BroadPhaseCollisionSystem(World* world, Vect2<uint16_t> windowSize);

    std::vector<PotentialCollisionPair>& HandleBroadPhaseCollisionSystem();

    bool GetCanDisplayGrid() const;
    bool GetCanHighlightGrid() const;
    void SetCanDisplayGrid(bool val);
    void SetCanHighlightGrid(bool val);
};
