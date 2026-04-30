#pragma once
#include "ecs/World.h"
#include "physics/CollisionCommon.h"

#include <cstdint>
#include <vector>
class CollisionResolutionSystem
{
  private:
    const uint8_t m_iterationCount = 4;

  public:
    CollisionResolutionSystem() = default;

    void ResolveCollisionOverlaps(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector, SolverBodies& sovlerBodies);
    void RefreshCollisionPenetrations(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector,
                                      SolverBodies& sovlerBodies);
    void ResolveCollisionImpulse(std::vector<CollisionCorrectionData>& collisionDataVector);
    void ResolveCollisions(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector, SolverBodies& sovlerBodies);
};
