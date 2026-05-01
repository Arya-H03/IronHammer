#include "NarrowPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "physics/CollisionCommon.h"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <immintrin.h>

void NarrowPhaseCollisionSystem::ScalarAABBCheck(const SolverBodies& solverBodies, uint16_t bodyAIndex, uint16_t bodyBIndex)
{
    float dx = solverBodies.posX[bodyBIndex] - solverBodies.posX[bodyAIndex];
    float dy = solverBodies.posY[bodyBIndex] - solverBodies.posY[bodyAIndex];

    float overlapX = (solverBodies.colliderHalfSizeX[bodyAIndex] + solverBodies.colliderHalfSizeX[bodyBIndex]) - std::abs(dx);
    float overlapY = (solverBodies.colliderHalfSizeY[bodyAIndex] + solverBodies.colliderHalfSizeY[bodyBIndex]) - std::abs(dy);

    float invMassA = solverBodies.inverseMasses[bodyAIndex];
    float invMassB = solverBodies.inverseMasses[bodyBIndex];

    if (overlapX <= 0.0f || overlapY <= 0.0f || invMassA + invMassB <= 0.0f) return;

    Vect2f normal;
    float penetration;

    if (overlapX < overlapY)
    {
        normal = (dx < 0.0f) ? Vect2f(-1, 0) : Vect2f(1, 0);
        penetration = std::max(0.01f, overlapX);
    }
    else
    {
        normal = (dy < 0.0f) ? Vect2f(0, -1) : Vect2f(0, 1);
        penetration = std::max(0.01f, overlapY);
    }

    m_collisionResults.normalX.push_back(normal.x);
    m_collisionResults.normalY.push_back(normal.y);
    m_collisionResults.penetration.push_back(penetration);
    m_collisionResults.solverBodyAIndex.push_back(bodyAIndex);
    m_collisionResults.solverBodyBIndex.push_back(bodyBIndex);
    m_collisionResults.invMassA.push_back(invMassA);
    m_collisionResults.invMassB.push_back(invMassB);
}

void NarrowPhaseCollisionSystem::SIMDAABBCheckGather(const SolverBodies& solverBodies, const SolverBodyPairs& solverBodyPairs,
                                                     size_t startIndex)
{
    const __m256 signMask = _mm256_set1_ps(-0.0f);
    const __m256 zero = _mm256_setzero_ps();

    __m128i idx16A = _mm_loadu_si128((__m128i*)&solverBodyPairs.bodyAIndices[startIndex]);
    __m256i bodyA = _mm256_cvtepu16_epi32(idx16A);

    __m128i idx16B = _mm_loadu_si128((__m128i*)&solverBodyPairs.bodyBIndices[startIndex]);
    __m256i bodyB = _mm256_cvtepu16_epi32(idx16B);

    __m256 aPosX = _mm256_i32gather_ps(solverBodies.posX.data(), bodyA, 4);
    __m256 aPosY = _mm256_i32gather_ps(solverBodies.posY.data(), bodyA, 4);

    __m256 bPosX = _mm256_i32gather_ps(solverBodies.posX.data(), bodyB, 4);
    __m256 bPosY = _mm256_i32gather_ps(solverBodies.posY.data(), bodyB, 4);

    __m256 aHalfX = _mm256_i32gather_ps(solverBodies.colliderHalfSizeX.data(), bodyA, 4);
    __m256 aHalfY = _mm256_i32gather_ps(solverBodies.colliderHalfSizeY.data(), bodyA, 4);

    __m256 bHalfX = _mm256_i32gather_ps(solverBodies.colliderHalfSizeX.data(), bodyB, 4);
    __m256 bHalfY = _mm256_i32gather_ps(solverBodies.colliderHalfSizeY.data(), bodyB, 4);

    __m256 aInvMass = _mm256_i32gather_ps(solverBodies.inverseMasses.data(), bodyA, 4);
    __m256 bInvMass = _mm256_i32gather_ps(solverBodies.inverseMasses.data(), bodyB, 4);

    __m256 dx = _mm256_sub_ps(bPosX, aPosX);
    __m256 dy = _mm256_sub_ps(bPosY, aPosY);

    __m256 absDx = _mm256_andnot_ps(signMask, dx);
    __m256 absDy = _mm256_andnot_ps(signMask, dy);

    __m256 overlapX = _mm256_sub_ps(_mm256_add_ps(aHalfX, bHalfX), absDx);
    __m256 overlapY = _mm256_sub_ps(_mm256_add_ps(aHalfY, bHalfY), absDy);

    __m256 maskX = _mm256_cmp_ps(overlapX, zero, _CMP_GT_OQ);
    __m256 maskY = _mm256_cmp_ps(overlapY, zero, _CMP_GT_OQ);

    __m256 massMask = _mm256_cmp_ps(_mm256_add_ps(aInvMass, bInvMass), zero, _CMP_GT_OQ);
    __m256 collisionMaskVec = _mm256_and_ps(_mm256_and_ps(maskX, maskY), massMask);
    uint32_t collisionMask = _mm256_movemask_ps(collisionMaskVec);

    __m256 useX = _mm256_cmp_ps(overlapX, overlapY, _CMP_LT_OQ);
    __m256 normalX = _mm256_blendv_ps(_mm256_set1_ps(1.0f), _mm256_set1_ps(-1.0f), _mm256_cmp_ps(dx, zero, _CMP_LT_OQ));
    __m256 finalNormalX = _mm256_blendv_ps(_mm256_setzero_ps(), normalX, useX);
    __m256 normalY = _mm256_blendv_ps(_mm256_set1_ps(1.0f), _mm256_set1_ps(-1.0f), _mm256_cmp_ps(dy, zero, _CMP_LT_OQ));
    __m256 finalNormalY = _mm256_blendv_ps(normalY, _mm256_setzero_ps(), useX);

    __m256 penetration = _mm256_blendv_ps(overlapY, overlapX, useX);
    penetration = _mm256_max_ps(penetration, _mm256_set1_ps(0.01f));

    alignas(32) float lanesNormalX[8];
    alignas(32) float lanesNormalY[8];
    alignas(32) float lanesPen[8];
    alignas(32) float lanesInvMassA[8];
    alignas(32) float lanesInvMassB[8];
    alignas(32) int32_t lanesBodyA[8];
    alignas(32) int32_t lanesBodyB[8];

    _mm256_store_ps(lanesNormalX, finalNormalX);
    _mm256_store_ps(lanesNormalY, finalNormalY);
    _mm256_store_ps(lanesPen, penetration);
    _mm256_store_ps(lanesInvMassA, aInvMass);
    _mm256_store_ps(lanesInvMassB, bInvMass);
    _mm256_store_si256((__m256i*)lanesBodyA, bodyA);
    _mm256_store_si256((__m256i*)lanesBodyB, bodyB);

    while (collisionMask)
    {
        int lane = std::countr_zero(collisionMask);

        m_collisionResults.normalX.push_back(lanesNormalX[lane]);
        m_collisionResults.normalY.push_back(lanesNormalY[lane]);
        m_collisionResults.penetration.push_back(lanesPen[lane]);
        m_collisionResults.invMassA.push_back(lanesInvMassA[lane]);
        m_collisionResults.invMassB.push_back(lanesInvMassB[lane]);
        m_collisionResults.solverBodyAIndex.push_back((uint16_t)lanesBodyA[lane]);
        m_collisionResults.solverBodyBIndex.push_back((uint16_t)lanesBodyB[lane]);

        collisionMask &= collisionMask - 1;
    }
}

CollisionResults& NarrowPhaseCollisionSystem::ProccessPotentialCollisonPairs(World* worldPtr, const SolverBodies& solverBodies,
                                                                             const SolverBodyPairs& solverBodyPairs)
{
    ZoneScopedN("CollisionNarrowPhase");

    m_collisionResults.Clear();
    size_t pairCount = solverBodyPairs.Count();
    m_collisionResults.Reserve(pairCount * 0.75f);

    size_t i = 0;
    for (; i + 7 < pairCount; i += 8)
    {
        SIMDAABBCheckGather(solverBodies, solverBodyPairs, i);
    }

    for (; i < pairCount; ++i)
    {
        ScalarAABBCheck(solverBodies, solverBodyPairs.bodyAIndices[i], solverBodyPairs.bodyBIndices[i]);
    }

    return m_collisionResults;
}
