#pragma once
#include "ecs/World.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"
#include "physics/CollisionEventSystem.h"

#include <cstdlib>
#include <vector>

class NarrowPhaseCollisionSystem
{
    friend class CollisionDebugger;

  private:
    CollisionEventSystem& m_collisionEventSystem;

    std::vector<CollisionCorrectionData> m_collisionPenetrationData;

    void AABBCheck(World* worldPtr, const PotentialCollisionPair& collisionPairData);

  public:
    NarrowPhaseCollisionSystem(CollisionEventSystem& collisionEventSystem) : m_collisionEventSystem(collisionEventSystem)
    {
    }

    std::vector<CollisionCorrectionData>& ProccessPotentialCollisonPairs(World* worldPtr, const std::vector<PotentialCollisionPair>& potentialPairs);
};
