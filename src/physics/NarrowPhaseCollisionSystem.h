#pragma once
#include <cstdlib>
#include <vector>
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"
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

    World& m_world;

    std::vector<CollisionData> m_collisionDataVector;

    // Axis Aligned Bounding Box
    void AABBCheck(Entity e1, Entity e2);

  public:

    NarrowPhaseCollisionSystem(World& world);

    std::vector<CollisionData>& ProccessPotentialCollisonPairs(const std::vector<PotentialCollisionPair>& potentialPairs);
};
