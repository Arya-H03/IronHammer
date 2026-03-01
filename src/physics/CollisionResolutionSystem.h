#pragma once
#include <vector>
#include "ecs/World.hpp"
#include "physics/NarrowPhaseCollisionSystem.h"
class CollisionResolutionSystem
{
  public:

    CollisionResolutionSystem() = default;

    void ResolveCollisionOverlaps(World* worldPtr, std::vector<CollisionData>& collisionDataVector);
    void ResolveCollisionImpluse(World* worldPtr, std::vector<CollisionData>& collisionDataVector);
    void ResolveCollisions(World* worldPtr, std::vector<CollisionData>& collisionDataVector);
};
