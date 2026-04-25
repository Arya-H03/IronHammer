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

    void ResolveCollisionOverlaps(std::vector<CollisionCorrectionData>& collisionDataVector);
    void RefreshCollisionPenetrations(std::vector<CollisionCorrectionData>& collisionDataVector);
    void ResolveCollisionImpulse(std::vector<CollisionCorrectionData>& collisionDataVector);
    void ResolveCollisions(std::vector<CollisionCorrectionData>& collisionDataVector);
};
