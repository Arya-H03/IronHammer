#pragma once

#include "Tracy.hpp"
#include "core/memory/InlineEntityList.h"
#include "ecs/World.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/system/ISystem.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <unordered_set>
#include <vector>

class CollisionEventSystem : ISetupSystem
{
    friend class CollisionDebugger;

  private:
    std::unordered_set<CollisionPair, CollisionPairHash> m_previousFramePairs;
    std::unordered_set<CollisionPair, CollisionPairHash> m_currentFramePairs;

    Query* m_collisionEventQueryPtr;

  public:
    void SetupSystem(World* worldPtr) override
    {
        m_collisionEventQueryPtr = worldPtr->Query<RequiredComponents<CCollisionEvent>>();
    }

    void HandleCollisionEvents(World* worldptr, const std::vector<CollisionCorrectionData>& collisionDataVector)
    {
        ZoneScopedN("CollisionEventSystem/HandleCollsionEvents");

        for (const auto& collisionData : collisionDataVector)
        {
            if (worldptr->TryGetComponent<CCollisionEvent>(collisionData.e1) ||
                worldptr->TryGetComponent<CCollisionEvent>(collisionData.e2))
            {
                m_currentFramePairs.emplace(CollisionPair(collisionData.e1, collisionData.e2));
            }
        }
        // stay and exit events
        for (auto& collisionPair : m_previousFramePairs)
        {
            CCollisionEvent* e1CollisionEventComp = worldptr->TryGetComponent<CCollisionEvent>(collisionPair.e1);
            CCollisionEvent* e2CollisionEventComp = worldptr->TryGetComponent<CCollisionEvent>(collisionPair.e2);

            if (m_currentFramePairs.contains(collisionPair))
            {
                if (e1CollisionEventComp)
                {
                    e1CollisionEventComp->stayEvents.Push(collisionPair.e2);
                }
                if (e2CollisionEventComp)
                {
                    e2CollisionEventComp->stayEvents.Push(collisionPair.e1);
                }
            }
            else
            {
                if (e1CollisionEventComp)
                {
                    e1CollisionEventComp->exitEvents.Push(collisionPair.e2);
                }
                if (e2CollisionEventComp)
                {
                    e2CollisionEventComp->exitEvents.Push(collisionPair.e1);
                }
            }
        }

        for (auto& collisionPair : m_currentFramePairs)
        {
            if (!m_previousFramePairs.contains(collisionPair))
            {
                CCollisionEvent* e1CollisionEventComp = worldptr->TryGetComponent<CCollisionEvent>(collisionPair.e1);
                CCollisionEvent* e2CollisionEventComp = worldptr->TryGetComponent<CCollisionEvent>(collisionPair.e2);

                if (e1CollisionEventComp)
                {
                    e1CollisionEventComp->enterEvents.Push(collisionPair.e2);
                }
                if (e2CollisionEventComp)
                {
                    e2CollisionEventComp->enterEvents.Push(collisionPair.e1);
                }
            }
        }

        std::swap(m_previousFramePairs, m_currentFramePairs);
    }

    // Call at the start of the Collision.h Update tick
    void ClearCollisionEvents(World* worldPtr)
    {
        ZoneScopedN("CollisionEventSystem/ClearCollisionEvents");
        m_currentFramePairs.clear();

        m_collisionEventQueryPtr->ForEach<CCollisionEvent>(
            [&](CCollisionEvent& collisionEventComp)
            {
                collisionEventComp.enterEvents.clear();
                collisionEventComp.stayEvents.clear();
                collisionEventComp.exitEvents.clear();
            });
    }
};
