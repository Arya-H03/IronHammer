#pragma once
#include "ecs/World.h"
#include "physics/CollisionCommon.h"

#include <cstdint>
class CollisionResolutionSystem
{
  private:
    const uint8_t m_iterationCount = 4;

  public:
    CollisionResolutionSystem() = default;

    void ResolveCollisionOverlaps(World* worldPtr, CollisionResults& collisionDataVector, SolverBodies& sovlerBodies);
    void ResolveCollisions(World* worldPtr, CollisionResults& collisionDataVector, SolverBodies& sovlerBodies);
};
