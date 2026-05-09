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
    const float m_cellSizeInv = 1.0f / m_cellSize;

    uint16_t m_gridCols, m_gridRows;

    std::vector<BroadPhaseGridCell<16>> m_broadPhaseGrid;
    std::vector<uint16_t> m_activeCellIndices;

    ThreadPool& m_threadPool;
    std::vector<BroadPhaseCellDataBuffer> m_cellDataBuffer;

    std::vector<BroadPhaseBodyPairBuffer> m_bodyPairBuffer;
    SolverBodyPairs m_solverBodyPairs;

    Query* m_broadPhaseQuery;
    SolverBodies& m_solverBodies;


    void FillCellWithThread(size_t threadIndex);
    void MergeCellDataBuffers();
    void FindCollisionPairsThread(size_t threadIndex);
    void MergeBodyPairBuffers();

    void FindCollisionPairsFromOverlaps();
    void FillCellsWithEntityOverlaps(World* worldPtr);

    bool CanCollidersContact(uint32_t colliderMaskA, uint32_t colliderMaskB, Layer colliderLayerA, Layer colliderLayerB);

  public:
    BroadPhaseCollisionSystem(SolverBodies& solverBodies, ThreadPool& threadPool);
    void SetupSystem(World* worldPtr) override;

    SolverBodyPairs& HandleBroadPhaseCollisionSystem(World* worldPtr);
};
