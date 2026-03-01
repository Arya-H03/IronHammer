#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>
#include <cstdlib>
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "Tracy.hpp"
#include "ecs/system/ISystem.h"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionDebugger.h"
#include "physics/CollisionResolutionSystem.h"
#include "physics/NarrowPhaseCollisionSystem.h"
#include "ecs/World.hpp"

class CollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:

    Vect2<uint16_t> m_windowSize;
    World* m_worldPtr;
    BroadPhaseCollisionSystem m_broadPhaseCollisionSystem;
    NarrowPhaseCollisionSystem m_narrowPhaseCollisionSystem;
    CollisionResolutionSystem m_collisionResolutionSystem;
    CollisionDebugger m_collisionDebugger;
    Query* m_collisionQuery;

    void CheckForScreenBorderCollision()
    {
        ZoneScopedN("CollisionSystem/CheckForScreenBorderCollision");
        for (auto& archetype : m_collisionQuery->GetMatchingArchetypes())
        {
            for (auto& chunk : archetype->GetChunks())
            {
                auto transformCompRow = chunk.GetComponentRow<CTransform>();
                auto rigidBodyCompRow = chunk.GetComponentRow<CRigidBody>();
                auto colliderCompRow = chunk.GetComponentRow<CCollider>();

                for (size_t i = 0; i < chunk.size; ++i)
                {
                    CTransform& transformComp = transformCompRow[i];
                    CRigidBody& rigidBodyComp = rigidBodyCompRow[i];
                    CCollider& colliderComp = colliderCompRow[i];

                    if (transformComp.position.y + colliderComp.offset.y - colliderComp.halfSize.y <= 0)
                    {
                        transformComp.position.y = colliderComp.halfSize.y - colliderComp.offset.y;
                        rigidBodyComp.velocity.y *= -1;
                    }
                    else if (transformComp.position.y + colliderComp.offset.y + colliderComp.halfSize.y >= m_windowSize.y)
                    {
                        transformComp.position.y = m_windowSize.y - colliderComp.halfSize.y - colliderComp.offset.y;
                        rigidBodyComp.velocity.y *= -1;
                    }

                    if (transformComp.position.x + colliderComp.offset.x - colliderComp.halfSize.x <= 0)
                    {
                        transformComp.position.x = colliderComp.halfSize.x - colliderComp.offset.x;
                        rigidBodyComp.velocity.x *= -1;
                    }

                    else if (transformComp.position.x + colliderComp.offset.x + colliderComp.halfSize.x >= m_windowSize.x)
                    {
                        transformComp.position.x = m_windowSize.x - colliderComp.halfSize.x - colliderComp.offset.x;
                        rigidBodyComp.velocity.x *= -1;
                    }
                }
            }
        }
    }

  public:

    const CollisionDebugger& GetCollsionDebugger() const { return m_collisionDebugger; }

    void SetupSystem(World* worldPtr) override
    {
        m_worldPtr = worldPtr;
        m_collisionQuery = m_worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
        m_broadPhaseCollisionSystem.SetupSystem(worldPtr);
    }

    CollisionSystem(Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize)
        , m_broadPhaseCollisionSystem(m_windowSize)
        , m_collisionDebugger(m_broadPhaseCollisionSystem, m_narrowPhaseCollisionSystem)
    {
    }

    void HandleCollisionSystem(World* worldPtr)
    {
        ZoneScoped;

        CheckForScreenBorderCollision();

        auto& potentialPairs = m_broadPhaseCollisionSystem.HandleBroadPhaseCollisionSystem(worldPtr);
        auto& collisionPairs = m_narrowPhaseCollisionSystem.ProccessPotentialCollisonPairs(worldPtr, potentialPairs);
        m_collisionResolutionSystem.ResolveCollisions(worldPtr, collisionPairs);
    }
};
