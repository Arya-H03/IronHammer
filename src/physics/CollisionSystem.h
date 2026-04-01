#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "ecs/query/Query.hpp"
#include "ecs/system/ISystem.h"
#include "ecs/World.hpp"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "editor/Viewport.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionEventSystem.h"
#include "physics/CollisionResolutionSystem.h"
#include "physics/NarrowPhaseCollisionSystem.h"
#include "Tracy.hpp"

#include <cstdint>
#include <cstdlib>
#include <SFML/Graphics/RenderWindow.hpp>

class CollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

private:
    Vect2<uint16_t> m_windowSize;

    CollisionEventSystem       m_collsionEventSystem;
    BroadPhaseCollisionSystem  m_broadPhaseCollisionSystem;
    NarrowPhaseCollisionSystem m_narrowPhaseCollisionSystem;
    CollisionResolutionSystem  m_collisionResolutionSystem;

    Query* m_collisionQuery;

    void CheckForScreenBorderCollision()
    {
        ZoneScopedN("CollisionSystem/CheckForScreenBorderCollision");

        m_collisionQuery->ForEach<CTransform, CRigidBody, CCollider>([](CTransform& transformComp, CRigidBody& rigidBodyComp,
                                                                        CCollider& colliderComp) {
            if (transformComp.position.y + colliderComp.offset.y - colliderComp.halfSize.y <= 0) {
                transformComp.position.y = colliderComp.halfSize.y - colliderComp.offset.y;
                rigidBodyComp.velocity.y *= -1;
            }
            else if (transformComp.position.y + colliderComp.offset.y + colliderComp.halfSize.y >= Viewport::GetSize().y) {
                transformComp.position.y = Viewport::GetSize().y - colliderComp.halfSize.y - colliderComp.offset.y;
                rigidBodyComp.velocity.y *= -1;
            }

            if (transformComp.position.x + colliderComp.offset.x - colliderComp.halfSize.x <= 0) {
                transformComp.position.x = colliderComp.halfSize.x - colliderComp.offset.x;
                rigidBodyComp.velocity.x *= -1;
            }

            else if (transformComp.position.x + colliderComp.offset.x + colliderComp.halfSize.x >= Viewport::GetSize().x) {
                transformComp.position.x = Viewport::GetSize().x - colliderComp.halfSize.x - colliderComp.offset.x;
                rigidBodyComp.velocity.x *= -1;
            }
        });
    }

public:
    void SetupSystem(World* worldPtr) override
    {
        m_collisionQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
        m_broadPhaseCollisionSystem.SetupSystem(worldPtr);
        m_collsionEventSystem.SetupSystem(worldPtr);
    }

    CollisionSystem(Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize), m_broadPhaseCollisionSystem({1500, 1500}),
          m_narrowPhaseCollisionSystem(m_collsionEventSystem)
    {
        SystemDebuggerHub::Instance().GetCollsionDebugger().RegisterCollisionSystem(this);
    }

    ~CollisionSystem() { SystemDebuggerHub::Instance().GetCollsionDebugger().UnRegisterCollsionSystem(); }

    void HandleCollisionSystem(World* worldPtr)
    {
        ZoneScoped;

        m_collsionEventSystem.ClearCollisionEvents(worldPtr);

        CheckForScreenBorderCollision();

        auto& potentialPairs = m_broadPhaseCollisionSystem.HandleBroadPhaseCollisionSystem(worldPtr);
        auto& collisionPairs = m_narrowPhaseCollisionSystem.ProccessPotentialCollisonPairs(worldPtr, potentialPairs);
        m_collisionResolutionSystem.ResolveCollisions(worldPtr, collisionPairs);
        m_collsionEventSystem.HandleCollisionEvents(worldPtr);
    }
};
