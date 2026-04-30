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
    NarrowPhaseSIMDBatch m_narrowPhaseBatch;

    std::vector<CollisionCorrectionData> m_collisionPenetrationData;

    void GatherNarrowPhaseBatch(const SolverBodies& solverBodies, const SolverBodyPairs& solverBodyPairs);
    void ScalarAABBCheck(size_t index);
    void SIMDAABBCheck(size_t startsIndex);
    void GenerateContactForSIMD(size_t index);

  public:
    NarrowPhaseCollisionSystem(CollisionEventSystem& collisionEventSystem) : m_collisionEventSystem(collisionEventSystem)
    {
    }

    std::vector<CollisionCorrectionData>& ProccessPotentialCollisonPairs(World* worldPtr, const SolverBodies& solverBodies,
                                                                         const SolverBodyPairs& solverBodyPairs);
};
