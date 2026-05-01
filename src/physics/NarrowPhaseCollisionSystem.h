#pragma once
#include "ecs/World.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"
#include "physics/CollisionEventSystem.h"

#include <cstddef>
#include <cstdlib>

class NarrowPhaseCollisionSystem
{
    friend class CollisionDebugger;

  private:
    CollisionEventSystem& m_collisionEventSystem;

    CollisionResults m_collisionResults;

    void ScalarAABBCheck(const SolverBodies& solverBodies, uint16_t bodyAIndex, uint16_t bodyBIndex);
    void SIMDAABBCheckGather(const SolverBodies& solverBodies, const SolverBodyPairs& solverBodyPairs, size_t startIndex);

  public:
    NarrowPhaseCollisionSystem(CollisionEventSystem& collisionEventSystem) : m_collisionEventSystem(collisionEventSystem)
    {
    }

    CollisionResults& ProccessPotentialCollisonPairs(World* worldPtr, const SolverBodies& solverBodies,
                                                     const SolverBodyPairs& solverBodyPairs);
};
