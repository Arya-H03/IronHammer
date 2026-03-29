#pragma once
#include "ecs/common/ECSCommon.h"
#include "ecs/World.hpp"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"

#include <cstdlib>
#include <unordered_set>
#include <vector>


class NarrowPhaseCollisionSystem
{
    friend class CollisionDebugger;

private:
    std::vector<CollisionData> m_collisionDataVector;

    std::unordered_set<CollisionPair, CollisionPairHash> m_previousFramePairs;
    std::unordered_set<CollisionPair, CollisionPairHash> m_currentFramePairs;

    // Axis Aligned Bounding Box
    void AABBCheck(World* worldPtr, Entity e1, Entity e2);

public:
    NarrowPhaseCollisionSystem() = default;
    std::vector<CollisionData>& ProccessPotentialCollisonPairs(World*                            worldPtr,
                                                               const std::vector<CollisionPair>& potentialPairs);
};
