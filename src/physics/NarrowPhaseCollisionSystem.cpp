#include "NarrowPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "physics/CollisionCommon.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <immintrin.h>

void NarrowPhaseCollisionSystem::GatherNarrowPhaseBatch(const SolverBodies& solverBodies, const SolverBodyPairs& solverBodyPairs)
{
    for (size_t i = 0; i < solverBodyPairs.Count(); ++i)
    {
        uint16_t bodyAIndex = solverBodyPairs.bodyAIndices[i];
        uint16_t bodyBIndex = solverBodyPairs.bodyBIndices[i];

        m_narrowPhaseBatch.Add(bodyAIndex, bodyBIndex, solverBodies.posX[bodyAIndex], solverBodies.posY[bodyAIndex],
                               solverBodies.colliderHalfSizeX[bodyAIndex], solverBodies.colliderHalfSizeY[bodyAIndex],
                               solverBodies.posX[bodyBIndex], solverBodies.posY[bodyBIndex], solverBodies.colliderHalfSizeX[bodyBIndex],
                               solverBodies.colliderHalfSizeY[bodyBIndex]);
    }
}

void NarrowPhaseCollisionSystem::ScalarAABBCheck(size_t index)
{
    float dx = m_narrowPhaseBatch.bPositionX[index] - m_narrowPhaseBatch.aPositionX[index];
    float dy = m_narrowPhaseBatch.bPositionY[index] - m_narrowPhaseBatch.aPositionY[index];

    float overlapX = (m_narrowPhaseBatch.aColliderHalfSizeX[index] + m_narrowPhaseBatch.bColliderHalfSizeX[index]) - std::abs(dx);
    float overlapY = (m_narrowPhaseBatch.aColliderHalfSizeY[index] + m_narrowPhaseBatch.bColliderHalfSizeY[index]) - std::abs(dy);

    if (overlapX <= 0.0f || overlapY <= 0.0f) return;

    Vect2f normal;
    float penetration;

    if (overlapX < overlapY)
    {
        normal = (dx < 0) ? Vect2f(-1, 0) : Vect2f(1, 0);
        penetration = overlapX;
    }
    else
    {
        normal = (dy < 0) ? Vect2f(0, -1) : Vect2f(0, 1);
        penetration = overlapY;
    }

    m_collisionPenetrationData.emplace_back(m_narrowPhaseBatch.aSolverBodyIndex[index], m_narrowPhaseBatch.bSolverBodyIndex[index], normal,
                                            penetration);
}

void NarrowPhaseCollisionSystem::SIMDAABBCheck(size_t startsIndex)
{
    const __m256 signMask = _mm256_set1_ps(-0.0f);
    const __m256 zero = _mm256_setzero_ps();

    // use aligned load maybe??
    __m256 aPositionX = _mm256_loadu_ps(&m_narrowPhaseBatch.aPositionX[startsIndex]);
    __m256 aPositionY = _mm256_loadu_ps(&m_narrowPhaseBatch.aPositionY[startsIndex]);
    __m256 aColliderHalfSizeX = _mm256_loadu_ps(&m_narrowPhaseBatch.aColliderHalfSizeX[startsIndex]);
    __m256 aColliderHalfSizeY = _mm256_loadu_ps(&m_narrowPhaseBatch.aColliderHalfSizeY[startsIndex]);

    __m256 bPositionX = _mm256_loadu_ps(&m_narrowPhaseBatch.bPositionX[startsIndex]);
    __m256 bPositionY = _mm256_loadu_ps(&m_narrowPhaseBatch.bPositionY[startsIndex]);
    __m256 bColliderHalfSizeX = _mm256_loadu_ps(&m_narrowPhaseBatch.bColliderHalfSizeX[startsIndex]);
    __m256 bColliderHalfSizeY = _mm256_loadu_ps(&m_narrowPhaseBatch.bColliderHalfSizeY[startsIndex]);

    // Distance
    __m256 dx = _mm256_sub_ps(bPositionX, aPositionX);
    __m256 dy = _mm256_sub_ps(bPositionY, aPositionY);

    // abs(Distance)
    __m256 absDx = _mm256_andnot_ps(signMask, dx);
    __m256 absDy = _mm256_andnot_ps(signMask, dy);

    // Overlap
    __m256 overlapX = _mm256_sub_ps(_mm256_add_ps(aColliderHalfSizeX, bColliderHalfSizeX), absDx);
    __m256 overlapY = _mm256_sub_ps(_mm256_add_ps(aColliderHalfSizeY, bColliderHalfSizeY), absDy);

    __m256 maskX = _mm256_cmp_ps(overlapX, zero, _CMP_GT_OQ);
    __m256 maskY = _mm256_cmp_ps(overlapY, zero, _CMP_GT_OQ);
    __m256 collisionMaskVec = _mm256_and_ps(maskX, maskY);
    uint32_t collisionMask = _mm256_movemask_ps(collisionMaskVec);

    alignas(32) float lanesDx[8], lanesDy[8], lanesOverlapX[8], lanesOverlapY[8];
    _mm256_store_ps(lanesDx, dx);
    _mm256_store_ps(lanesDy, dy);
    _mm256_store_ps(lanesOverlapX, overlapX);
    _mm256_store_ps(lanesOverlapY, overlapY);

    while (collisionMask)
    {
        int lane = std::countr_zero(collisionMask);
        size_t index = startsIndex + lane;

        float ox = lanesOverlapX[lane];
        float oy = lanesOverlapY[lane];

        Vect2f normal;
        float penetration;

        if (ox < oy)
        {
            normal = (lanesDx[lane] < 0) ? Vect2f(-1, 0) : Vect2f(1, 0);
            penetration = ox;
        }
        else
        {
            normal = (lanesDy[lane] < 0) ? Vect2f(0, -1) : Vect2f(0, 1);
            penetration = oy;
        }

        m_collisionPenetrationData.emplace_back(m_narrowPhaseBatch.aSolverBodyIndex[index], m_narrowPhaseBatch.bSolverBodyIndex[index],
                                                normal, penetration);
        collisionMask &= collisionMask - 1;
    }
}

void NarrowPhaseCollisionSystem::GenerateContactForSIMD(size_t index)
{
    float dx = m_narrowPhaseBatch.bPositionX[index] - m_narrowPhaseBatch.aPositionX[index];
    float dy = m_narrowPhaseBatch.bPositionY[index] - m_narrowPhaseBatch.aPositionY[index];

    float overlapX = (m_narrowPhaseBatch.aColliderHalfSizeX[index] + m_narrowPhaseBatch.bColliderHalfSizeX[index]) - std::abs(dx);
    float overlapY = (m_narrowPhaseBatch.aColliderHalfSizeY[index] + m_narrowPhaseBatch.bColliderHalfSizeY[index]) - std::abs(dy);

    Vect2f normal;
    float penetration;

    if (overlapX < overlapY)
    {
        normal = (dx < 0) ? Vect2f(-1, 0) : Vect2f(1, 0);
        penetration = overlapX;
    }
    else
    {
        normal = (dy < 0) ? Vect2f(0, -1) : Vect2f(0, 1);
        penetration = overlapY;
    }

    m_collisionPenetrationData.emplace_back(m_narrowPhaseBatch.aSolverBodyIndex[index], m_narrowPhaseBatch.bSolverBodyIndex[index], normal,
                                            penetration);
}


std::vector<CollisionCorrectionData>& NarrowPhaseCollisionSystem::ProccessPotentialCollisonPairs(World* worldPtr,
                                                                                                 const SolverBodies& solverBodies,
                                                                                                 const SolverBodyPairs& solverBodyPairs)
{
    ZoneScopedN("CollisionNarrowPhase");

    m_collisionPenetrationData.clear();
    m_narrowPhaseBatch.Clear();
    GatherNarrowPhaseBatch(solverBodies, solverBodyPairs);
    size_t count = m_narrowPhaseBatch.Count();
    size_t i = 0;

    for (; i + 7 < count; i += 8)
    {
        SIMDAABBCheck(i);
    }
    for (; i < count; ++i)
    {
        ScalarAABBCheck(i);
    }

    return m_collisionPenetrationData;
}
