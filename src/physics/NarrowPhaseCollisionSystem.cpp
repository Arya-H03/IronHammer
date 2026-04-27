#include "NarrowPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "physics/CollisionCommon.h"

#include <cstdlib>


void NarrowPhaseCollisionSystem::AABBCheck(World* worldPtr, const PotentialCollisionPair& collisionPairData)
{
    {
        ZoneScopedN("NarrowPhaseSystem/AABB_Checks/Calculations");
        {

            // AABB check
            Vect2f e1Center = collisionPairData.e1TransformPtr->position + collisionPairData.e1ColliderPtr->offset;
            Vect2f e2Center = collisionPairData.e2TransformPtr->position + collisionPairData.e2ColliderPtr->offset;

            Vect2f distance = e2Center - e1Center;
            Vect2f distanceAbs = distance.Abs();
            Vect2f overlap = (collisionPairData.e1ColliderPtr->halfSize + collisionPairData.e2ColliderPtr->halfSize) - distanceAbs;

            // Will Collide
            if (overlap.x > 0 && overlap.y > 0)
            {
                Vect2f normal;
                float penetration;

                if (overlap.x < overlap.y)
                {
                    normal = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0);
                    penetration = overlap.x;
                }
                else
                {
                    normal = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1);
                    penetration = overlap.y;
                }

                {
                    ZoneScopedN("NarrowPhaseSystem/AABB_Checks/CreatePenetrationData");
                    m_collisionPenetrationData.emplace_back(collisionPairData.e1, collisionPairData.e2, normal, penetration,
                                                            collisionPairData.e1TransformPtr, collisionPairData.e1RigidBodyPtr,
                                                            collisionPairData.e1ColliderPtr, collisionPairData.e2TransformPtr,
                                                            collisionPairData.e2RigidBodyPtr, collisionPairData.e2ColliderPtr);
                }
            }
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
