#pragma once
#include "core/utils/Threadpool.h"
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
    const float m_cellSize = 8;

    uint16_t m_gridCols, m_gridRows;

    std::vector<BroadPhaseGridCell<16>> m_broadPhaseGrid;

    ThreadPool& m_threadPool;
    std::vector<BroadPhaseThreadBuffer> m_threadBuffers;

    SolverBodyPairs m_solverBodyPairs;

    Query* m_broadPhaseQuery;
    SolverBodies& m_solverBodies;


    void FillCellWithThread(size_t threadIndex);
    void MergeThreadBuffers();

    void FillCellsWithEntityOverlaps(World* worldPtr);
    void FindCollisionPairsFromOverlaps();

    bool CanCollidersContact(uint32_t colliderMaskA, uint32_t colliderMaskB, Layer colliderLayerA, Layer colliderLayerB);

  public:
    BroadPhaseCollisionSystem(SolverBodies& solverBodies, ThreadPool& threadPool);
    void SetupSystem(World* worldPtr) override;

    SolverBodyPairs& HandleBroadPhaseCollisionSystem(World* worldPtr);
};
