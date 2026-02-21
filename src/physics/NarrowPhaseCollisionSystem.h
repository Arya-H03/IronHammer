#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/Components.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "physics/BroadPhaseCollisionSystem.h"
#include <cstdlib>
#include <unordered_set>
#include <vector>

struct CollisionPair
{
    Entity e1, e2;
    Vect2f overlapSize;
};

class NarrowPhaseCollisonSystem
{
    friend class CollisionDebugger;

  private:

    EntityManager& m_entityManager;
    std::vector<CollisionPair> m_collisionPair;

  public:

    NarrowPhaseCollisonSystem(EntityManager& entityManager) : m_entityManager(entityManager) { }

     std::vector<CollisionPair>& ProccessPotentialCollisonPairs(const std::unordered_set<PotentialCollisionPair, PotentialPairHash>& potentialPairs)
    {
        m_collisionPair.clear();

        for (auto& potentialPair : potentialPairs)
        {
            Entity e1 = potentialPair.e1;
            Entity e2 = potentialPair.e2;

            CTransform* e1Transform = m_entityManager.TryGetComponent<CTransform>(e1);
            CTransform* e2Transform = m_entityManager.TryGetComponent<CTransform>(e2);

            CCollider* e1Collider = m_entityManager.TryGetComponent<CCollider>(e1);
            CCollider* e2Collider = m_entityManager.TryGetComponent<CCollider>(e2);

            Vect2f distanceVect =
                ((e1Transform->position + e1Collider->offset) - (e2Transform->position + e2Collider->offset)).Abs();

            bool xCollide = distanceVect.x <= (e1Collider->halfSize.x + e2Collider->halfSize.x);
            bool yCollide = distanceVect.y <= (e1Collider->halfSize.y + e2Collider->halfSize.y);

            if (xCollide && yCollide)
            {
                m_collisionPair.emplace_back(e1, e2, e1Collider->halfSize + e2Collider->halfSize - distanceVect);
            }
        }

        return m_collisionPair;
    }
};
