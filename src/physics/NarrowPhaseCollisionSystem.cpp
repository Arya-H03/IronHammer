#include "NarrowPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "physics/CollisionCommon.h"

#include <cstdint>
#include <cstdlib>

bool NarrowPhaseCollisionSystem::CanColliderContact(CCollider* collider1, CCollider* collider2)
{
    uint32_t e1Bit = static_cast<uint32_t>(collider1->layer);
    uint32_t e2Bit = static_cast<uint32_t>(collider2->layer);

    bool e1WantsE2 = (collider1->mask & e2Bit) != 0;
    bool e2WantsE1 = (collider2->mask & e1Bit) != 0;

    if (!e1WantsE2 || !e2WantsE1)
        return false;
    else
        return true;
}

void NarrowPhaseCollisionSystem::AABBCheck(World* worldPtr, Entity e1, Entity e2)
{
    CCollider* e1Collider = worldPtr->TryGetComponent<CCollider>(e1);
    CCollider* e2Collider = worldPtr->TryGetComponent<CCollider>(e2);

    if (!CanColliderContact(e1Collider, e2Collider)) return;

    CRigidBody* e1Rb = worldPtr->TryGetComponent<CRigidBody>(e1);
    CRigidBody* e2Rb = worldPtr->TryGetComponent<CRigidBody>(e2);

    CTransform* e1Transform = worldPtr->TryGetComponent<CTransform>(e1);
    CTransform* e2Transform = worldPtr->TryGetComponent<CTransform>(e2);

    // AABB check
    Vect2f e1Center = e1Transform->position + e1Collider->offset;
    Vect2f e2Center = e2Transform->position + e2Collider->offset;
    Vect2f distance = e2Center - e1Center;
    Vect2f distanceAbs = distance.Abs();

    bool xCollide = distanceAbs.x <= (e1Collider->halfSize.x + e2Collider->halfSize.x);
    bool yCollide = distanceAbs.y <= (e1Collider->halfSize.y + e2Collider->halfSize.y);

    // Will Collide
    if (xCollide && yCollide)
    {
        Vect2f overlap = (e1Collider->halfSize + e2Collider->halfSize) - distanceAbs;

        Vect2f lastFrameE1Center = e1Rb->previousPosition + e1Collider->offset;
        Vect2f lastFrameE2Center = e2Rb->previousPosition + e2Collider->offset;
        Vect2f lastFrameDistance = lastFrameE2Center - lastFrameE1Center;

        bool lastXCollide = std::abs(lastFrameDistance.x) <= (e1Collider->halfSize.x + e2Collider->halfSize.x);
        bool lastYCollide = std::abs(lastFrameDistance.y) <= (e1Collider->halfSize.y + e2Collider->halfSize.y);

        Vect2f normal;
        float penetration;

        // Was overlapping on X only, last frame -> Collision entered on Y
        if (lastXCollide && !lastYCollide)
        {
            normal = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1); // up : down
            penetration = overlap.y;
        }
        // Was overlapping on Y only, last frame -> Collision entered on X
        else if (!lastXCollide && lastYCollide)
        {
            normal = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0); // right : left
            penetration = overlap.x;
        }
        // Wasn't overlapping last frame -> Collision entered on X and Y
        else
        {
            if (overlap.x < overlap.y)
            {
                normal = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0); // right : left
                penetration = overlap.x;
            }
            else
            {
                normal = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1); // up : down
                penetration = overlap.y;
            }
        }
        m_collisionPenetrationData.emplace_back(e1, e2, normal, penetration);
        m_collisionEventSystem.CreateCollisionPair(e1, e2);
    }
}

std::vector<CollisionCorrectionData>&
NarrowPhaseCollisionSystem::ProccessPotentialCollisonPairs(World* worldPtr, const std::vector<CollisionPair>& potentialPairs)
{
    m_collisionPenetrationData.clear();
    {
        ZoneScopedN("NarrowPhaseSystem/AABB_Checks");
        for (auto& potentialPair : potentialPairs)
        {
            Entity e1 = potentialPair.e1;
            Entity e2 = potentialPair.e2;

            AABBCheck(worldPtr, e1, e2);
        }
    }

    return m_collisionPenetrationData;
}
