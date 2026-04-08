#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/system/ISystem.h"
#include "physics/CollisionCommon.h"

#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstdint>
#include <unordered_set>
#include <vector>

struct Cell
{
    Vect2<int> coord;
    Vect2f pos;
    std::vector<Entity> overlapingEntities;

    Cell() = default;
    Cell(Vect2<int> c, Vect2f p) : coord(c), pos(p) {}
};

class BroadPhaseCollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const float m_cellSize = 32;
    const float m_cellRadius = std::sqrt((m_cellSize * m_cellSize) / 2);

    uint16_t m_cellPerRow, m_cellPerCol;

    Vect2<uint16_t> m_windowSize;
    std::vector<Cell> m_grid;
    std::vector<Entity> m_gridDisplayEntities;

    std::unordered_set<CollisionPair, CollisionPairHash> m_uniquePotentialPairsSet;
    std::vector<CollisionPair> m_uniquePotentialPairsVector;

    Query* m_broadPhaseQuery;

    // Flags
    bool m_canDisplayGrid = false;
    bool m_canHighlightGrid = false;

    void PopulateGrid(World* worldPtr);
    void ClearAllCells();
    void FillCellsWithOverlappingEntities(World* worldPtr);
    void FindUniqueCollisionPairs();

  public:
    BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize);
    void SetupSystem(World* worldPtr) override;

    std::vector<CollisionPair>& HandleBroadPhaseCollisionSystem(World* worldPtr);

    bool GetCanDisplayGrid() const;
    bool GetCanHighlightGrid() const;
    void SetCanDisplayGrid(World* worldPtr, bool val);
    void SetCanHighlightGrid(World* worldPtr, bool val);
};
