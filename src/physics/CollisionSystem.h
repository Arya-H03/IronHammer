#pragma once
#include "Tracy.hpp"
#include "backward.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Time.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/query/Query.hpp"
#include "ecs/system/ISystem.h"
#include "editor/Viewport.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "game/GameComponents.hpp"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionEventSystem.h"
#include "physics/CollisionResolutionSystem.h"
#include "physics/NarrowPhaseCollisionSystem.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <cstdint>
#include <cstdlib>

// To do:
//
//  - Remake the CollisionEventSystem
//
//  - Explore resting contact
//
//  - Maybe a steering / avoidance on the flowfield
//    can help tp reduce density.
//
//  - Better pair filtering on broad phase?
//
//  - Compare how many collision pairs are generated
//
//    by broad phase Vs confirmed by narrow phase.
//  - Make the AABB check cheaper.
//
//  - Mutlithread broad & narrow phases

class CollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const uint8_t SUBSTEP_COUNT = 8;

    Vect2<uint16_t> m_windowSize;

    CollisionEventSystem m_collsionEventSystem;
    BroadPhaseCollisionSystem m_broadPhaseCollisionSystem;
    NarrowPhaseCollisionSystem m_narrowPhaseCollisionSystem;
    CollisionResolutionSystem m_collisionResolutionSystem;

    Query* m_updateCollisionQueryPtr;
    Query* m_updatePositionQueryPtr;

    void UpdatePositions(float dt)
    {
        m_updatePositionQueryPtr->ForEach<CTransform, CRigidBody, CMovement, CFlowFieldAgent>(
            [&](CTransform& transform, CRigidBody& rigidBody, CMovement& movement, CFlowFieldAgent& flowFieldAgent)
            {
                if (rigidBody.isStatic) return;

                Vect2f current = transform.position;
                transform.position += (transform.position - transform.previousPosition) + flowFieldAgent.flowDir * 100 * dt * dt;
                transform.previousPosition = current;
            });
    }

    void CheckForScreenBorderCollision()
    {
        ZoneScopedN("CollisionSystem/CheckForScreenBorderCollision");

        m_updateCollisionQueryPtr->ForEach<CTransform, CRigidBody, CCollider>(
            [](CTransform& transform, CRigidBody& rigidBody, CCollider& collider)
            {
                if (rigidBody.isStatic) return;

                if (transform.position.y + collider.offset.y - collider.halfSize.y <= 0.0f)
                {
                    transform.position.y = collider.halfSize.y - collider.offset.y;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.y *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }
                else if (transform.position.y + collider.offset.y + collider.halfSize.y >= Viewport::GetSize().y)
                {
                    transform.position.y = Viewport::GetSize().y - collider.halfSize.y - collider.offset.y;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.y *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }

                if (transform.position.x + collider.offset.x - collider.halfSize.x <= 0.0f)
                {
                    transform.position.x = collider.halfSize.x - collider.offset.x;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.x *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }
                else if (transform.position.x + collider.offset.x + collider.halfSize.x >= Viewport::GetSize().x)
                {
                    transform.position.x = Viewport::GetSize().x - collider.halfSize.x - collider.offset.x;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.x *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }
            });
    }

  public:
    void SetupSystem(World* worldPtr) override
    {
        m_updatePositionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CRigidBody,CFlowFieldAgent>>();
        m_updateCollisionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
        m_broadPhaseCollisionSystem.SetupSystem(worldPtr);
        m_collsionEventSystem.SetupSystem(worldPtr);
    }

    CollisionSystem(World* worldPtr, Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize), m_broadPhaseCollisionSystem({1500, 1500}), m_narrowPhaseCollisionSystem(m_collsionEventSystem)
    {
        SetupSystem(worldPtr);
        SystemDebuggerHub::Instance().GetCollsionDebugger().RegisterCollisionSystem(this);
    }

    ~CollisionSystem()
    {
        SystemDebuggerHub::Instance().GetCollsionDebugger().UnRegisterCollsionSystem();
    }

    void HandleCollisionSystem(World* worldPtr, float dt)
    {
        ZoneScoped;

        m_collsionEventSystem.ClearCollisionEvents(worldPtr);

        float substepDt = dt / SUBSTEP_COUNT;
        for (size_t i = 0; i < SUBSTEP_COUNT; ++i)
        {
            {
                ZoneScopedN("CollisionSystem/UpdatePosition");
                UpdatePositions(substepDt);
            }

            {
                ZoneScopedN("CollisionSystem/BroadPhaseCollision");
                auto& potentialPairs = m_broadPhaseCollisionSystem.HandleBroadPhaseCollisionSystem(worldPtr);

                {
                    ZoneScopedN("CollisionSystem/NarrowPhaseCollision");
                    auto& collisionData = m_narrowPhaseCollisionSystem.ProccessPotentialCollisonPairs(worldPtr, potentialPairs);

                    {
                        ZoneScopedN("CollisionSystem/CollisionResolution");
                        m_collisionResolutionSystem.ResolveCollisions(worldPtr, collisionData);
                    }
                }
            }

            {
                ZoneScopedN("CollisionSystem/BorderCollision");
                CheckForScreenBorderCollision();
            }
        }

        m_updatePositionQueryPtr->ForEach<CTransform, CRigidBody>(
            [&](CTransform& transform, CRigidBody& rb)
            {
                if (rb.isStatic) return;
                transform.previousPosition = transform.position - (transform.position - transform.previousPosition);
            });

        {

            ZoneScopedN("CollisionSystem/HandleCollisionEvents");
            m_collsionEventSystem.HandleCollisionEvents(worldPtr);
        }
    }
};
