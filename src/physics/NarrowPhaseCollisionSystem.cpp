#include "NarrowPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/common/ECSCommon.h"
#include "physics/CollisionCommon.h"

#include <cstdlib>


void NarrowPhaseCollisionSystem::AABBCheck(World* worldPtr, const PotentialCollisionPair& collisionPairData)
{
    Entity e1 = collisionPairData.e1;
    Entity e2 = collisionPairData.e2;
    CCollider* e1Collider;
    CCollider* e2Collider;
    CRigidBody* e1Rb;
    CRigidBody* e2Rb;
    CTransform* e1Transform;
    CTransform* e2Transform;

    {
        ZoneScopedN("NarrowPhaseSystem/AABB_Checks/ComponentFetching");
        e1Collider = collisionPairData.e1ColliderPtr;
        e2Collider = collisionPairData.e2ColliderPtr;
        e1Rb = collisionPairData.e1RigidBodyPtr;
        e2Rb = collisionPairData.e2RigidBodyPtr;
        e1Transform = collisionPairData.e1TransformPtr;
        e2Transform = collisionPairData.e2TransformPtr;
    }

    {
        ZoneScopedN("NarrowPhaseSystem/AABB_Checks/Calculation");

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

            Vect2f lastFrameE1Center = e1Transform->previousPosition + e1Collider->offset;
            Vect2f lastFrameE2Center = e2Transform->previousPosition + e2Collider->offset;
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
            m_collisionPenetrationData.emplace_back(e1, e2, normal, penetration, e1Transform, e1Rb, e2Transform, e2Rb);
            m_collisionEventSystem.CreateCollisionPair(e1, e2);
        }
    }
}

std::vector<CollisionCorrectionData>&
NarrowPhaseCollisionSystem::ProccessPotentialCollisonPairs(World* worldPtr, const std::vector<PotentialCollisionPair>& potentialPairs)
{
    m_collisionPenetrationData.clear();
    for (auto& potentialPair : potentialPairs)
    {
        AABBCheck(worldPtr, potentialPair);
    }

    return m_collisionPenetrationData;
}
