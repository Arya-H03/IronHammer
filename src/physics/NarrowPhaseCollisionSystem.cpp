#include "NarrowPhaseCollisionSystem.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"
#include <cstdlib>

NarrowPhaseCollisionSystem::NarrowPhaseCollisionSystem(EntityManager& entityManager) : m_entityManager(entityManager) { }

void NarrowPhaseCollisionSystem::AABBCheck(Entity e1, Entity e2)
{
    {
        ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone1");

        {
            ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone1.1");
            CTransform* e1Transform = m_entityManager.TryGetComponent<CTransform>(e1);
            CTransform* e2Transform = m_entityManager.TryGetComponent<CTransform>(e2);

            CCollider* e1Collider = m_entityManager.TryGetComponent<CCollider>(e1);
            CCollider* e2Collider = m_entityManager.TryGetComponent<CCollider>(e2);

            CRigidBody* e1Rb = m_entityManager.TryGetComponent<CRigidBody>(e1);
            CRigidBody* e2Rb = m_entityManager.TryGetComponent<CRigidBody>(e2);

            {
                ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone1.2");

                // AABB check
                Vect2f e1Center = e1Transform->position + e1Collider->offset;
                Vect2f e2Center = e2Transform->position + e2Collider->offset;
                Vect2f distance = e2Center - e1Center;
                Vect2f distanceAbs = distance.Abs();

                bool xCollide = distanceAbs.x <= (e1Collider->halfSize.x + e2Collider->halfSize.x);
                bool yCollide = distanceAbs.y <= (e1Collider->halfSize.y + e2Collider->halfSize.y);

                {
                    ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/Zone2");
                    // Entites will collide
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

                        if (lastXCollide && !lastYCollide) // Was overlapping on X only, last frame -> Collision entered on Y
                        {
                            normal = (distance.y < 0) ? Vect2f(0, -1) : Vect2f(0, 1); // up : down
                            penetration = overlap.y;
                        }
                        else if (!lastXCollide && lastYCollide) // Was overlapping on Y only, last frame -> Collision entered on X
                        {
                            normal = (distance.x < 0) ? Vect2f(-1, 0) : Vect2f(1, 0); // right : left
                            penetration = overlap.x;
                        }
                        else // Wasn't overlapping last frame -> Collision entered on X and Y
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
                        m_collisionDataVector.emplace_back(e1, e2, normal, penetration);
                    }
                }
            }
        }
    }
}

std::vector<CollisionData>& NarrowPhaseCollisionSystem::ProccessPotentialCollisonPairs(const std::vector<PotentialCollisionPair>& potentialPairs)
{
    {
        ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs");
        m_collisionDataVector.clear();

        for (auto& potentialPair : potentialPairs)
        {
            Entity e1 = potentialPair.e1;
            Entity e2 = potentialPair.e2;

            {
                ZoneScopedN("NarrowPhaseSystem/ProccessPotentialCollisonPairs/AABBCheck");
                AABBCheck(e1, e2);
            }
        }

        return m_collisionDataVector;
    }
}
