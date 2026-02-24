#pragma once
#include <vector>
#include "ecs/entity/EntityManager.hpp"
#include "physics/NarrowPhaseCollisionSystem.h"
class CollisionResolutionSystem
{
  private:

    EntityManager& m_entityManager;

  public:

    CollisionResolutionSystem(EntityManager& entityManager);

    void ResolveCollisionOverlaps(std::vector<CollisionData>& collisionDataVector);
    void ResolveCollisionImpluse(std::vector<CollisionData>& collisionDataVector);
    void ResolveCollisions(std::vector<CollisionData>& collisionDataVector);


};
