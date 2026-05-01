#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/system/ISystem.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstdint>
#include <vector>


class BroadPhaseCollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const float m_cellSize = 12;
    const float m_cellRadius = std::sqrt((m_cellSize * m_cellSize) / 2);

    uint16_t m_gridCols, m_gridRows;
    Vect2<uint16_t> m_windowSize;

    std::vector<BroadGridCell<16>> m_broadPhaseGrid;
    std::vector<uint16_t> m_activeBroadGridCellIndices;
    SolverBodyPairs m_solverBodyPairs;

    Query* m_broadPhaseQuery;

    void ClearAllCells();
    void FillCellsWithOverlappingEntities(World* worldPtr, SolverBodies& solverBodies);
    void FindCollisionPairs(const SolverBodies& solverBodies);

    bool CanCollidersContact(uint32_t colliderMaskA, uint32_t colliderMaskB, Layer colliderLayerA, Layer colliderLayerB);

  public:
    BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize);
    void SetupSystem(World* worldPtr) override;

    SolverBodyPairs& HandleBroadPhaseCollisionSystem(World* worldPtr, SolverBodies& solverBodies);
};
