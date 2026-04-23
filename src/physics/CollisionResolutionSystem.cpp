#include "CollisionResolutionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "physics/PhysicsComponents.hpp"

void CollisionResolutionSystem::ResolveCollisionOverlaps(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector)
{
    ZoneScopedN("CollisionResolutionSystem/ResolveCollisionOverlaps");

    for (auto& collisionData : collisionDataVector)
    {
        float invMass1 = collisionData.e1RigidBodyPtr->inverseMass;
        float invMass2 = collisionData.e2RigidBodyPtr->inverseMass;
        float invMassSum = invMass1 + invMass2;
        if (invMassSum == 0.0f) continue;

        collisionData.e1TransformPtr->position -= collisionData.normal * collisionData.penetration * invMass1 / invMassSum * 0.5f;
        collisionData.e2TransformPtr->position += collisionData.normal * collisionData.penetration * invMass2 / invMassSum * 0.5f;
    }
}

void CollisionResolutionSystem::ResolveCollisionImpulse(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector)
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

void CollisionResolutionSystem::ResolveCollisions(World* worldPtr, std::vector<CollisionCorrectionData>& collisionDataVector)
{
    ResolveCollisionOverlaps(worldPtr, collisionDataVector);
    // ResolveCollisionImpulse(worldPtr, collisionDataVector);
}
