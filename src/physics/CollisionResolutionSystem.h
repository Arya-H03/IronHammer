#pragma once
#include "ecs/World.h"
#include "physics/CollisionCommon.h"

#include <vector>
class CollisionResolutionSystem
{
  public:
    CollisionResolutionSystem() = default;

    void ResolveCollisionOverlaps(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector);
    void ResolveCollisionImpulse(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector);
    void ResolveCollisions(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector);
};
