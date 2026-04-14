#pragma once
#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/query/Query.hpp"
#include "ecs/system/ISystem.h"
#include "editor/Viewport.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionEventSystem.h"
#include "physics/CollisionResolutionSystem.h"
#include "physics/NarrowPhaseCollisionSystem.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>
#include <cstdlib>

// To do:
//
//  - Remake the CollisionEventSystem
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
    const uint8_t SUBSTEP_COUNT = 1;

    Vect2<uint16_t> m_windowSize;

    CollisionEventSystem m_collsionEventSystem;
    BroadPhaseCollisionSystem m_broadPhaseCollisionSystem;
    NarrowPhaseCollisionSystem m_narrowPhaseCollisionSystem;
    CollisionResolutionSystem m_collisionResolutionSystem;

    Query* m_updateCollisionQueryPtr;
    Query* m_updatePositionQueryPtr;

    void SavePreviousPositions()
    {
        m_updatePositionQueryPtr->ForEach<CTransform, CRigidBody>(
            [](CTransform& t, CRigidBody& rb)
            {
                if (!rb.isStatic) rb.previousPosition = t.position;
            });
    }

    void UpdatePositions(float dt)
    {
        m_updatePositionQueryPtr->ForEach<CTransform, CRigidBody, CMovement>(
            [&](CTransform& t, CRigidBody& rb, CMovement& mv)
            {
                if (rb.isStatic) return;
                rb.previousPosition = t.position;
                t.position += rb.velocity * dt * mv.speed;
            });
    }

    void CheckForScreenBorderCollision()
    {
        ZoneScopedN("CollisionSystem/CheckForScreenBorderCollision");

        m_updateCollisionQueryPtr->ForEach<CTransform, CRigidBody, CCollider>(
            [](CTransform& t, CRigidBody& rb, CCollider& col)
            {
                if (rb.isStatic) return;

                if (t.position.y + col.offset.y - col.halfSize.y <= 0.0f)
                {
                    t.position.y = col.halfSize.y - col.offset.y;
                    rb.velocity.y *= -rb.bounciness;
                }
                else if (t.position.y + col.offset.y + col.halfSize.y >= Viewport::GetSize().y)
                {
                    t.position.y = Viewport::GetSize().y - col.halfSize.y - col.offset.y;
                    rb.velocity.y *= -rb.bounciness;
                }

                if (t.position.x + col.offset.x - col.halfSize.x <= 0.0f)
                {
                    t.position.x = col.halfSize.x - col.offset.x;
                    rb.velocity.x *= -rb.bounciness;
                }
                else if (t.position.x + col.offset.x + col.halfSize.x >= Viewport::GetSize().x)
                {
                    t.position.x = Viewport::GetSize().x - col.halfSize.x - col.offset.x;
                    rb.velocity.x *= -rb.bounciness;
                }
            });
    }

  public:
    void SetupSystem(World* worldPtr) override
    {
        m_updatePositionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CRigidBody, CMovement>>();
        m_updateCollisionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
        m_broadPhaseCollisionSystem.SetupSystem(worldPtr);
        m_collsionEventSystem.SetupSystem(worldPtr);
    }

    CollisionSystem(Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize), m_broadPhaseCollisionSystem({1500, 1500}), m_narrowPhaseCollisionSystem(m_collsionEventSystem)
    {
        SystemDebuggerHub::Instance().GetCollsionDebugger().RegisterCollisionSystem(this);
    }

    ~CollisionSystem()
    {
        SystemDebuggerHub::Instance().GetCollsionDebugger().UnRegisterCollsionSystem();
    }

    void HandleCollisionSystem(World* worldPtr, float dt)
    {
        ZoneScoped;


        SavePreviousPositions();
        const float subStepDt = Time::DeltaTime() / SUBSTEP_COUNT;

        m_collsionEventSystem.ClearCollisionEvents(worldPtr);

        for (uint8_t i = 0; i < SUBSTEP_COUNT; ++i)
        {
            {
                ZoneScopedN("CollisionSystem/UpdatePosition");
                UpdatePositions(subStepDt);
            }

            {
                ZoneScopedN("CollisionSystem/BorderCollision");
                CheckForScreenBorderCollision();
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
                ZoneScopedN("CollisionSystem/HandleCollisionEvents");
                m_collsionEventSystem.HandleCollisionEvents(worldPtr);
            }
        }
    }
};
