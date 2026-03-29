#include "NarrowPhaseCollisionSystem.h"

#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include "physics/CollisionCommon.h"
#include "Tracy.hpp"

#include <cstdint>
#include <cstdlib>
#include <utility>

void NarrowPhaseCollisionSystem::AABBCheck(World* worldPtr, Entity e1, Entity e2)
{
    {
        // ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone1");
        {
            // ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone1.1");
            CCollider* e1Collider = worldPtr->TryGetComponent<CCollider>(e1);
            CCollider* e2Collider = worldPtr->TryGetComponent<CCollider>(e2);

            uint32_t e1Bit = static_cast<uint32_t>(e1Collider->layer);
            uint32_t e2Bit = static_cast<uint32_t>(e2Collider->layer);

            bool e1WantsE2 = (e1Collider->mask & e2Bit) != 0;
            bool e2WantsE1 = (e2Collider->mask & e1Bit) != 0;

            if (!e1WantsE2 || !e2WantsE1) return;

            CRigidBody* e1Rb = worldPtr->TryGetComponent<CRigidBody>(e1);
            CRigidBody* e2Rb = worldPtr->TryGetComponent<CRigidBody>(e2);

            CTransform* e1Transform = worldPtr->TryGetComponent<CTransform>(e1);
            CTransform* e2Transform = worldPtr->TryGetComponent<CTransform>(e2);

            {
                // ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone1.2");

                // AABB check
                Vect2f e1Center    = e1Transform->position + e1Collider->offset;
                Vect2f e2Center    = e2Transform->position + e2Collider->offset;
                Vect2f distance    = e2Center - e1Center;
                Vect2f distanceAbs = distance.Abs();

                bool xCollide = distanceAbs.x <= (e1Collider->halfSize.x + e2Collider->halfSize.x);
                bool yCollide = distanceAbs.y <= (e1Collider->halfSize.y + e2Collider->halfSize.y);

                {
                    // ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone2");
                    //  Entites will collide
                    if (xCollide && yCollide) {
                        Vect2f overlap = (e1Collider->halfSize + e2Collider->halfSize) - distanceAbs;

                        Vect2f lastFrameE1Center = e1Rb->previousPosition + e1Collider->offset;
                        Vect2f lastFrameE2Center = e2Rb->previousPosition + e2Collider->offset;
                        Vect2f lastFrameDistance = lastFrameE2Center - lastFrameE1Center;

                        bool lastXCollide =
                            std::abs(lastFrameDistance.x) <= (e1Collider->halfSize.x + e2Collider->halfSize.x);
                        bool lastYCollide =
                            std::abs(lastFrameDistance.y) <= (e1Collider->halfSize.y + e2Collider->halfSize.y);

                        Vect2f normal;
                        float  penetration;

                        if (lastXCollide && !lastYCollide) // Was overlapping on X only, last frame -> Collision entered on Y
                        {
                            normal      = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1); // up : down
                            penetration = overlap.y;
                        }
                        else if (!lastXCollide &&
                                 lastYCollide) // Was overlapping on Y only, last frame -> Collision entered on X
                        {
                            normal      = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0); // right : left
                            penetration = overlap.x;
                        }
                        else // Wasn't overlapping last frame -> Collision entered on X and Y
                        {
                            if (overlap.x < overlap.y) {
                                normal      = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0); // right : left
                                penetration = overlap.x;
                            }
                            else {
                                normal      = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1); // up : down
                                penetration = overlap.y;
                            }
                        }
                        m_collisionDataVector.emplace_back(e1, e2, normal, penetration);
                        m_currentFramePairs.emplace(CollisionPair(e1, e2));
                    }
                }
            }
        }
    }
}

std::vector<CollisionData>&
NarrowPhaseCollisionSystem::ProccessPotentialCollisonPairs(World* worldPtr, const std::vector<CollisionPair>& potentialPairs)
{
    {
        // ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs");
        m_collisionDataVector.clear();
        m_currentFramePairs.clear();

        for (auto& potentialPair : potentialPairs) {
            Entity e1 = potentialPair.e1;
            Entity e2 = potentialPair.e2;

            {
                // ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/AABBCheck");
                AABBCheck(worldPtr, e1, e2);
            }
        }

        for (auto& collisionPair : m_previousFramePairs) {
            if (m_currentFramePairs.contains(collisionPair)) {
                worldPtr->CreateEntityNoReturn(CCollisionStay(collisionPair.e1, collisionPair.e2));
            }
            else worldPtr->CreateEntityNoReturn(CCollisionExit(collisionPair.e1, collisionPair.e2));
        }
        for (auto& collisionPair : m_currentFramePairs) {
            if (!m_previousFramePairs.contains(collisionPair)) {
                worldPtr->CreateEntityNoReturn(CCollisionEnter(collisionPair.e1, collisionPair.e2));
            }
        }
        std::swap(m_previousFramePairs, m_currentFramePairs);

        return m_collisionDataVector;
    }
}
