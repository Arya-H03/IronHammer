#pragma once
#include "ecs/World.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"
#include "physics/CollisionEventSystem.h"

#include <cstddef>
#include <cstdlib>
#include <vector>

class NarrowPhaseCollisionSystem
{
    friend class CollisionDebugger;

  private:
    CollisionEventSystem& m_collisionEventSystem;

    std::vector<CollisionCorrectionData> m_collisionPenetrationData;

    void ScalarAABBCheck(const NarrowPhaseSIMDBatch& batch,size_t index);
    void SIMDAABBCheck(const NarrowPhaseSIMDBatch& batch, size_t startsIndex);
    void GenerateContactForSIMD(const NarrowPhaseSIMDBatch& batchm, size_t index);

  public:
    NarrowPhaseCollisionSystem(CollisionEventSystem& collisionEventSystem) : m_collisionEventSystem(collisionEventSystem)
    {
    }

    std::vector<CollisionCorrectionData>& ProccessPotentialCollisonPairs(World* worldPtr, const NarrowPhaseSIMDBatch& batch,
                                                                         const std::vector<SolverBody>& solverBodies);
};
