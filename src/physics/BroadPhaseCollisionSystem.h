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
#include <utility>
#include <vector>


class BroadPhaseCollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const float m_cellSize = 8;
    const float m_cellSizeInv = 1.0f / m_cellSize;

    uint16_t m_gridCols, m_gridRows;

    std::vector<BroadPhaseCellDataEntry> m_mergedCellData;
    std::vector<BroadPhaseCellDataEntry> m_tempCellData;
    std::vector<uint16_t> m_coutingCells;
    std::vector<std::pair<size_t, size_t>> m_threadIndexBounds;

    ThreadPool& m_threadPool;
    std::vector<BroadPhaseCellDataBuffer> m_cellDataBuffer;

    std::vector<BroadPhaseBodyPairBuffer> m_bodyPairBuffer;
    SolverBodyPairs m_solverBodyPairs;

    Query* m_broadPhaseQuery;
    SolverBodies& m_solverBodies;


    void GatherCellDataIntoBuffer(size_t threadIndex);
    void MergeCellDataBuffers();
    void SortCellData();
    void FindThreadWorkloadBounds();

    void GatherCollisionPairsIntoBuffer(size_t threadIndex);
    void MergeCollisionPairBuffers();

    bool CanCollidersContact(uint32_t colliderMaskA, uint32_t colliderMaskB, Layer colliderLayerA, Layer colliderLayerB);

  public:
    BroadPhaseCollisionSystem(SolverBodies& solverBodies, ThreadPool& threadPool);
    void SetupSystem(World* worldPtr) override;

    SolverBodyPairs& HandleBroadPhaseCollisionSystem(World* worldPtr);
};
