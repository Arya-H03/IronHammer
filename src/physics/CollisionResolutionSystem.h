#pragma once
#include <vector>
#include "ecs/World.hpp"
#include "physics/NarrowPhaseCollisionSystem.h"
class CollisionResolutionSystem
{
  private:

    World& m_world;

  public:

    CollisionResolutionSystem(World& world);

    void ResolveCollisionOverlaps(std::vector<CollisionData>& collisionDataVector);
    void ResolveCollisionImpluse(std::vector<CollisionData>& collisionDataVector);
    void ResolveCollisions(std::vector<CollisionData>& collisionDataVector);


};
