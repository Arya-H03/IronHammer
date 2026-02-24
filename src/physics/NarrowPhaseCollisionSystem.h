#pragma once
#include <cstdlib>
#include <vector>
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityManager.hpp"
#include "physics/BroadPhaseCollisionSystem.h"

struct CollisionData
{
    Entity e1, e2;
    Vect2f normal;
    float penetration;
};

class NarrowPhaseCollisionSystem
{
    friend class CollisionDebugger;

  private:

    EntityManager& m_entityManager;
    std::vector<CollisionData> m_collisionDataVector;

    // Axis Aligned Bounding Box
    void AABBCheck(Entity e1, Entity e2);

  public:

    NarrowPhaseCollisionSystem(EntityManager& entityManager);

    std::vector<CollisionData>& ProccessPotentialCollisonPairs(const std::vector<PotentialCollisionPair>& potentialPairs);
};
