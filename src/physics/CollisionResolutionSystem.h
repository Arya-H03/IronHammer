#pragma once
#include <vector>
#include "ecs/World.hpp"
#include "physics/CollisionCommon.h"
class CollisionResolutionSystem
{
  public:

    CollisionResolutionSystem() = default;

    void ResolveCollisionOverlaps(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector);
    void ResolveCollisionImpluse(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector);
    void ResolveCollisions(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector);
};
