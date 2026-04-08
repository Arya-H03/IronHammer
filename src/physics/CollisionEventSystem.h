#pragma once

#include "ecs/common/ECSCommon.h"
#include "ecs/system/ISystem.h"
#include "ecs/World.hpp"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <unordered_set>

class CollisionEventSystem : ISetupSystem
{
private:
    std::unordered_set<CollisionPair, CollisionPairHash> m_previousFramePairs;
    std::unordered_set<CollisionPair, CollisionPairHash> m_currentFramePairs;

    Query* m_enterCollisionQuery;
    Query* m_stayCollisionQuery;
    Query* m_exitCollisionQuery;

public:
    void SetupSystem(World* worldPtr) override
    {
        m_enterCollisionQuery = worldPtr->Query<RequiredComponents<CCollisionEnter>>();
        m_stayCollisionQuery  = worldPtr->Query<RequiredComponents<CCollisionStay>>();
        m_exitCollisionQuery  = worldPtr->Query<RequiredComponents<CCollisionExit>>();
    }

    void CreateCollisionPair(Entity e1, Entity e2) { m_currentFramePairs.emplace(CollisionPair(e1, e2)); }

    // Check me later:
    // Call either after ResolutionSystem or maybe after??
    void HandleCollisionEvents(World* worldptr)
    {
        // stay and exit events
        for (auto& collisionpair : m_previousFramePairs) {
            if (m_currentFramePairs.contains(collisionpair)) {
                worldptr->CreateEntityNoReturn(CCollisionStay(collisionpair.e1, collisionpair.e2));
            }
            else worldptr->CreateEntityNoReturn(CCollisionExit(collisionpair.e1, collisionpair.e2));
        }

        // enter events
        for (auto& collisionpair : m_currentFramePairs) {
            if (!m_previousFramePairs.contains(collisionpair)) {
                worldptr->CreateEntityNoReturn(CCollisionEnter(collisionpair.e1, collisionpair.e2));
            }
        }

        std::swap(m_previousFramePairs, m_currentFramePairs);
    }

    // Call at the start of the Collision.h Update tick
    void ClearCollisionEvents(World* worldPtr)
    {
        m_currentFramePairs.clear();

        m_enterCollisionQuery->ForEachWithEntity([&](Entity entity) { worldPtr->DestroyEntity(entity); });
        m_exitCollisionQuery->ForEachWithEntity([&](Entity entity) { worldPtr->DestroyEntity(entity); });
        m_stayCollisionQuery->ForEachWithEntity([&](Entity entity) { worldPtr->DestroyEntity(entity); });
    }
};
