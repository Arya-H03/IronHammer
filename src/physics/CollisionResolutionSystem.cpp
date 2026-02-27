#include "CollisionResolutionSystem.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"

CollisionResolutionSystem::CollisionResolutionSystem(World* world) : m_worldPtr(world) { }

void CollisionResolutionSystem::ResolveCollisionOverlaps(std::vector<CollisionData>& collisionDataVector)
{
    {
        ZoneScopedN("CollisionResolutionSystem/ResolveCollisionOverlaps");

        for (auto& collisionData : collisionDataVector)
        {
            CTransform* transform1 = m_worldPtr->TryGetComponent<CTransform>(collisionData.e1);
            CTransform* transform2 = m_worldPtr->TryGetComponent<CTransform>(collisionData.e2);
            CRigidBody* rb1 = m_worldPtr->TryGetComponent<CRigidBody>(collisionData.e1);
            CRigidBody* rb2 = m_worldPtr->TryGetComponent<CRigidBody>(collisionData.e2);

            float invMass1 = rb1->inverseMass;
            float invMass2 = rb2->inverseMass;
            float invMassSum = invMass1 + invMass2;
            if (invMassSum == 0.0f) continue;

            transform1->position -= collisionData.normal * collisionData.penetration * (invMass1 / invMassSum);
            transform2->position += collisionData.normal * collisionData.penetration * (invMass2 / invMassSum);
        }
    }
}

void CollisionResolutionSystem::ResolveCollisionImpluse(std::vector<CollisionData>& collisionDataVector)
{
    {
        ZoneScopedN("CollisionResolutionSystem/ResolveCollisionImpluse");

        for (auto& collisionData : collisionDataVector)
        {
            CRigidBody* rb1 = m_worldPtr->TryGetComponent<CRigidBody>(collisionData.e1);
            CRigidBody* rb2 = m_worldPtr->TryGetComponent<CRigidBody>(collisionData.e2);

            float invMass1 = rb1->inverseMass;
            float invMass2 = rb2->inverseMass;
            float invMassSum = invMass1 + invMass2;
            if (invMassSum == 0.0f) continue;

            Vect2f relativeVelocity = rb2->velocity - rb1->velocity;
            float velAlongNormal = relativeVelocity.DotProduct(collisionData.normal);
            if (velAlongNormal > 0) continue;

            float e = std::min(rb1->bounciness, rb2->bounciness);
            float j = -(1.0f + e) * velAlongNormal;
            j /= invMassSum;

            Vect2f impulse = collisionData.normal * j;

            rb1->velocity -= impulse * invMass1;
            rb2->velocity += impulse * invMass2;
        }
    }
}

void CollisionResolutionSystem::ResolveCollisions(std::vector<CollisionData>& collisionDataVector)
{
    ResolveCollisionOverlaps(collisionDataVector);
    ResolveCollisionImpluse(collisionDataVector);
}
