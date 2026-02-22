#pragma once
#include <cstdlib>
#include <unordered_set>
#include <vector>
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityManager.hpp"
#include "physics/BroadPhaseCollisionSystem.h"

struct CollisionPair
{
    Entity e1, e2;
    Vect2f overlapSize;
};

class NarrowPhaseCollisionSystem
{
    friend class CollisionDebugger;

  private:

    EntityManager& m_entityManager;
    std::vector<CollisionPair> m_collisionPair;

  public:

    NarrowPhaseCollisionSystem(EntityManager& entityManager);

    std::vector<CollisionPair>& ProccessPotentialCollisonPairs(const std::vector<PotentialCollisionPair>& potentialPairs);
};
