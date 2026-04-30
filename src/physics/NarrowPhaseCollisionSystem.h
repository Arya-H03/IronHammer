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
    static constexpr size_t CHUNK_SIZE = 32;
    CollisionEventSystem& m_collisionEventSystem;

    std::vector<CollisionCorrectionData> m_collisionPenetrationData;

    void GatherNarrowPhaseBatch(const SolverBodies& solverBodies, const SolverBodyPairs& solverBodyPairs, NarrowPhaseSIMDBatch& batch,
                                size_t startIndex);
    void ScalarAABBCheck(NarrowPhaseSIMDBatch& batch, size_t index);
    void SIMDAABBCheck(NarrowPhaseSIMDBatch& batch, size_t startsIndex);
    void GenerateContactForSIMD(size_t index);

  public:
    NarrowPhaseCollisionSystem(CollisionEventSystem& collisionEventSystem) : m_collisionEventSystem(collisionEventSystem)
    {
    }

    std::vector<CollisionCorrectionData>& ProccessPotentialCollisonPairs(World* worldPtr, const SolverBodies& solverBodies,
                                                                         const SolverBodyPairs& solverBodyPairs);
};
