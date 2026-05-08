#include "CollisionResolutionSystem.h"

#include "Tracy.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "physics/CollisionCommon.h"

#include <cstddef>
#include <cstdint>
#include <vector>

void CollisionResolutionSystem::ResolveCollisionOverlaps(World* worldPtr, CollisionResults& collisionResults, SolverBodies& sovlerBodies)
{
    // ZoneScopedN("CollisionResolutionSystem/ResolveCollisionOverlaps");

    const float percent = 1.f;

    for (size_t i = 0; i < collisionResults.Count(); ++i)
    {
        float invMassA = collisionResults.invMassA[i];
        float invMassB = collisionResults.invMassB[i];
        float invMassSum = invMassA + invMassB;

        Vect2f correction =
            Vect2f(collisionResults.normalX[i], collisionResults.normalY[i]) * ((collisionResults.penetration[i] * percent) / invMassSum);

        sovlerBodies.posX[collisionResults.solverBodyAIndex[i]] -= correction.x * invMassA;
        sovlerBodies.posY[collisionResults.solverBodyAIndex[i]] -= correction.y * invMassA;

        sovlerBodies.posX[collisionResults.solverBodyBIndex[i]] += correction.x * invMassB;
        sovlerBodies.posY[collisionResults.solverBodyBIndex[i]] += correction.y * invMassB;

    }
}

void CollisionResolutionSystem::ResolveCollisions(World* worldPtr, CollisionResults& collisionResults, SolverBodies& solverBodies)
{
    ZoneScopedN("CollisionResolution");

    for (uint8_t i = 0; i < m_iterationCount; ++i)
    {
        ResolveCollisionOverlaps(worldPtr, collisionResults, solverBodies);
    }

}
