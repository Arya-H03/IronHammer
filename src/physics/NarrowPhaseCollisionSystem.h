#pragma once
#include "ecs/common/ECSCommon.h"
#include "ecs/World.hpp"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"
#include "physics/CollisionEventSystem.h"
#include "physics/PhysicsComponents.hpp"

#include <cstdlib>
#include <vector>

class NarrowPhaseCollisionSystem
{
    friend class CollisionDebugger;

private:
    CollisionEventSystem& m_collisionEventSystem;

    std::vector<CollisionCorrectionData> m_collisionPenetrationData;

    bool CanColliderContact(CCollider* collider1, CCollider* collider2);
    // Axis Aligned Bounding Box Check
    void AABBCheck(World* worldPtr, Entity e1, Entity e2);

public:
    NarrowPhaseCollisionSystem(CollisionEventSystem& collisionEventSystem) : m_collisionEventSystem(collisionEventSystem) {}

    std::vector<CollisionCorrectionData>& ProccessPotentialCollisonPairs(World*                            worldPtr,
                                                                         const std::vector<CollisionPair>& potentialPairs);
};
