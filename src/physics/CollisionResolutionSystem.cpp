#include "CollisionResolutionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <cstdint>
#include <vector>

void CollisionResolutionSystem::ResolveCollisionOverlaps(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector)
{
    ZoneScopedN("CollisionResolutionSystem/ResolveCollisionOverlaps");

    const float percent = .25f;
    const float slop = 0.01f;

    for (auto& collisionData : collisionDataVector)
    {
        float invMass1 = collisionData.e1RigidbodyPtr->inverseMass;
        float invMass2 = collisionData.e2RigidbodyPtr->inverseMass;
        float invMassSum = invMass1 + invMass2;
        if (invMassSum == 0.0f) continue;

        float penetration = std::max(collisionData.penetration - slop, 0.0f);
        if (penetration <= 0.0f) continue;

        Vect2f correction = collisionData.normal * ((penetration * percent) / invMassSum);
        collisionData.e1TransformPtr->position -= correction * invMass1;
        collisionData.e2TransformPtr->position += correction * invMass2;
    }
}

void CollisionResolutionSystem::RefreshCollisionPenetrations(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector)
{
    ZoneScopedN("CollisionResolutionSystem/RefreshCollisionPenetrations");

    for (auto& collisionData : collisionDataVector)
    {
        Vect2f e1Center = collisionData.e1TransformPtr->position + collisionData.e1ColliderPtr->offset;
        Vect2f e2Center = collisionData.e2TransformPtr->position + collisionData.e2ColliderPtr->offset;

        Vect2f distance = e2Center - e1Center;
        Vect2f distanceAbs = distance.Abs();
        Vect2f overlap = (collisionData.e1ColliderPtr->halfSize + collisionData.e2ColliderPtr->halfSize) - distanceAbs;

        if (overlap.x > 0 && overlap.y > 0)
        {
            if (overlap.x < overlap.y)
            {
                collisionData.normal = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0);
                collisionData.penetration = overlap.x;
            }
            else
            {
                collisionData.normal = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1);
                collisionData.penetration = overlap.y;
            }
        }
        else
        {
            collisionData.penetration = 0;
        }
    }
}

void CollisionResolutionSystem::ResolveCollisions(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector)
{
    for (uint8_t i = 0; i < m_iterationCount; ++i)
    {
        ResolveCollisionOverlaps(worldPtr, collisionDataVector);
        if (i == m_iterationCount - 1) break;
        RefreshCollisionPenetrations(worldPtr, collisionDataVector);
    }
}

// Deprecated
void CollisionResolutionSystem::ResolveCollisionImpulse(std::vector<CollisionCorrectionData>& collisionDataVector)
{
    // {
    //     ZoneScopedN("CollisionResolutionSystem/ResolveCollisionImpluse");

    //     for (auto& collisionData : collisionDataVector)
    //     {
    //         CRigidBody* rb1 = collisionData.e1RigidBodyPtr;
    //         CRigidBody* rb2 = collisionData.e2RigidBodyPtr;

    //         float invMass1 = rb1->inverseMass;
    //         float invMass2 = rb2->inverseMass;
    //         float invMassSum = invMass1 + invMass2;
    //         if (invMassSum == 0.0f) continue;

    //         Vect2f relativeVelocity = rb2->velocity - rb1->velocity;
    //         float velAlongNormal = relativeVelocity.DotProduct(collisionData.normal);
    //         if (velAlongNormal > 0) continue;

    //         float e = std::min(rb1->bounciness, rb2->bounciness);
    //         float j = -(1.0f + e) * velAlongNormal;
    //         j /= invMassSum;

    //         const float maxImpulse = 10.0f;
    //         j = std::min(j, maxImpulse);

    //         Vect2f impulse = collisionData.normal * j;

    //         rb1->velocity -= impulse * invMass1;
    //         rb2->velocity += impulse * invMass2;
    //     }
    // }
}
