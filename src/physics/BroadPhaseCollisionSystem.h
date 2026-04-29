#pragma once
#include "core/memory/InlineVector.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/system/ISystem.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>


class BroadPhaseCollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const float m_cellSize = 24;
    const float m_cellRadius = std::sqrt((m_cellSize * m_cellSize) / 2);

    uint16_t m_gridCols, m_gridRows;
    Vect2<uint16_t> m_windowSize;

    std::vector<InlineVector<BroadPhaseCellData, 12>> m_broadPhaseGrid;
    NarrowPhaseSIMDBatch m_narrowPhasebatch;
    size_t m_lastFrameBatchCount = 0;

    Query* m_broadPhaseQuery;

    bool m_canDisplayGrid = false;
    bool m_canHighlightGrid = false;

    void ClearAllCells();
    void FillCellsWithOverlappingEntities(World* worldPtr, std::vector<SolverBody>& solverBodies);
    void FindCollisionPairs(const std::vector<SolverBody>& solverBodies);

    bool CanCollidersContact(uint32_t colliderMaskA, uint32_t colliderMaskB, Layer colliderLayerA, Layer colliderLayerB);

  public:
    BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize);
    void SetupSystem(World* worldPtr) override;

    NarrowPhaseSIMDBatch& HandleBroadPhaseCollisionSystem(World* worldPtr, std::vector<SolverBody>& solverBodies);

    bool GetCanDisplayGrid() const;
    bool GetCanHighlightGrid() const;
    void SetCanDisplayGrid(World* worldPtr, bool val);
    void SetCanHighlightGrid(World* worldPtr, bool val);
};
