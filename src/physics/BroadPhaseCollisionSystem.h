#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/system/ISystem.h"
#include "physics/CollisionCommon.h"

#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstdint>
#include <vector>


class BroadPhaseCollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const float m_cellSize = 32;
    const float m_cellRadius = std::sqrt((m_cellSize * m_cellSize) / 2);

    uint16_t m_gridCols, m_gridRows;
    Vect2<uint16_t> m_windowSize;

    std::vector<BroadPhaseCellData> m_flatGridData;
    std::vector<BroadPhaseCellData> m_flatGridBuffer;
    std::vector<CollisionPair> m_collisionPairs;

    Query* m_broadPhaseQuery;

    // Flags
    bool m_canDisplayGrid = false;
    bool m_canHighlightGrid = false;

    void ClearAllCells();
    void FillCellsWithOverlappingEntities(World* worldPtr);
    void FindCollisionPairs();

    bool CanCollidersContact(CCollider* collider1, CCollider* collider2);

    void CountSort(std::vector<BroadPhaseCellData>& toSort, int exponent);
    void RadixSortGridData(std::vector<BroadPhaseCellData>& toSort);

  public:
    BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize);
    void SetupSystem(World* worldPtr) override;

    std::vector<CollisionPair>& HandleBroadPhaseCollisionSystem(World* worldPtr);

    bool GetCanDisplayGrid() const;
    bool GetCanHighlightGrid() const;
    void SetCanDisplayGrid(World* worldPtr, bool val);
    void SetCanHighlightGrid(World* worldPtr, bool val);
};
