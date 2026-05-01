#include "BroadPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/memory/InlineVector.h"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "editor/Viewport.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <immintrin.h>
#include <vector>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize) : m_windowSize(windowSize)
{
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_gridRows = (Viewport::GetSize().y / m_cellSize) + 5;
    m_gridCols = (Viewport::GetSize().x / m_cellSize) + 5;
    m_broadPhaseGrid.resize(m_gridRows * m_gridCols);
    m_activeBroadGridCellIndices.reserve(m_gridCols * m_gridRows);
    m_solverBodyPairs.Reserve(100000);

    m_broadPhaseQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
}

void BroadPhaseCollisionSystem::FillCellsWithOverlappingEntities(World* worldPtr, SolverBodies& solverBodies)
{
    ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");

    for (uint16_t cellIndex : m_activeBroadGridCellIndices)
    {
        m_broadPhaseGrid[cellIndex].Clear();
    }
    m_activeBroadGridCellIndices.clear();

    for (uint16_t k = 0; k < solverBodies.Count(); ++k)
    {
        Vect2f center{solverBodies.posX[k], solverBodies.posY[k]};
        Vect2f colliderHalfSize{solverBodies.colliderHalfSizeX[k], solverBodies.colliderHalfSizeY[k]};
        Vect2<uint8_t> topLeftCell = ((center - (colliderHalfSize)) / m_cellSize).FloorCast<uint8_t>();
        Vect2<uint8_t> bottomRightCell = ((center + (colliderHalfSize)) / m_cellSize).FloorCast<uint8_t>();

        for (int j = topLeftCell.y; j <= bottomRightCell.y; ++j)
        {
            for (int i = topLeftCell.x; i <= bottomRightCell.x; ++i)
            {
                if (j < 0 || j >= (int)m_gridRows || i < 0 || i >= (int)m_gridCols) continue;

                uint16_t index = j * m_gridCols + i;
                auto& broadGridCell = m_broadPhaseGrid[index];
                if (broadGridCell.Size() == 0)
                {
                    m_activeBroadGridCellIndices.push_back(index);
                }

                broadGridCell.Add(solverBodies.entites[k], k, topLeftCell, solverBodies.colliderMasks[k], solverBodies.colliderLayers[k]);
            }
        }
    }
}

void BroadPhaseCollisionSystem::FindCollisionPairs(const SolverBodies& solverBodies)
{
    ZoneScopedN("BroadPhaseSystem/FindCollisionPairs");

    for (uint16_t currentCellIndex : m_activeBroadGridCellIndices)
    {
        const auto& broadGridCell = m_broadPhaseGrid[currentCellIndex];
        const size_t count = broadGridCell.Size();
        if (count < 2) continue;

        for (size_t i = 0; i < count; ++i)
        {
            const uint16_t solverBodyAIndex = broadGridCell.solverBodyIndices.list[i];
            const uint8_t aMinX = broadGridCell.minCellX.list[i];
            const uint8_t aMinY = broadGridCell.minCellY.list[i];
            const uint32_t maskA = broadGridCell.collisionMasks.list[i];
            const uint32_t layerA = broadGridCell.collisionLayers.list[i];
            const Entity entityA = broadGridCell.entities.list[i];

            for (size_t j = i + 1; j < count; ++j)
            {
                const uint16_t sharedMinX = std::max(aMinX, broadGridCell.minCellX.list[j]);
                const uint16_t sharedMinY = std::max(aMinY, broadGridCell.minCellY.list[j]);
                const uint16_t ownerCellIndex = sharedMinY * m_gridCols + sharedMinX;
                if (currentCellIndex != ownerCellIndex) continue;

                uint32_t maskB = broadGridCell.collisionMasks.list[j];
                uint32_t layerB = broadGridCell.collisionLayers.list[j];

                if (!CanCollidersContact(maskA, maskB, static_cast<Layer>(layerA), static_cast<Layer>(layerB))) continue;

                const Entity entityB = broadGridCell.entities.list[j];
                const uint16_t solverBodyBIndex = broadGridCell.solverBodyIndices.list[j];
                const bool aIsLower = entityA.id < entityB.id;
                uint16_t sA = aIsLower ? solverBodyAIndex : solverBodyBIndex;
                uint16_t sB = aIsLower ? solverBodyBIndex : solverBodyAIndex;

                m_solverBodyPairs.AddPair(sA, sB);
            }
        }
    }
}

bool BroadPhaseCollisionSystem::CanCollidersContact(uint32_t colliderMaskA, uint32_t colliderMaskB, Layer colliderLayerA,
                                                    Layer colliderLayerB)
{
    uint32_t bitA = static_cast<uint32_t>(colliderLayerA);
    uint32_t bitB = static_cast<uint32_t>(colliderLayerB);

    bool aWantsB = (colliderMaskA & bitB) != 0;
    bool bWantsA = (colliderMaskB & bitA) != 0;

    return aWantsB && bWantsA;
}

SolverBodyPairs& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr, SolverBodies& solverBodies)
{
    ZoneScopedN("CollisionBroadPhase");

    FillCellsWithOverlappingEntities(worldPtr, solverBodies);

    m_solverBodyPairs.Clear();
    FindCollisionPairs(solverBodies);

    return m_solverBodyPairs;
}

// void BroadPhaseCollisionSystem::FindCollisionPairs(const SolverBodies& solverBodies)
// {
//     ZoneScopedN("BroadPhaseSystem/FindCollisionPairs");

//     const __m256i gridCols = _mm256_set1_epi16(m_gridCols);
//     const __m256i zero = _mm256_setzero_si256();
//     {
//         ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/ForeachActiveCell1");
//         for (uint16_t currentCellIndex : m_activeBroadGridCellIndices)
//         {
//             const auto& broadGridCell = m_broadPhaseGrid[currentCellIndex];
//             const size_t count = broadGridCell.Size();
//             if (count < 2) continue;

//             const __m256i currentCell = _mm256_set1_epi16(currentCellIndex);
//             {
//                 ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/ForeachActiveCell2");
//                 for (size_t i = 0; i < count; ++i)
//                 {
//                     const uint16_t solverBodyAIndex = broadGridCell.solverBodyIndices.list[i];
//                     const uint8_t aMinX = broadGridCell.minCellX.list[i];
//                     const uint8_t aMinY = broadGridCell.minCellY.list[i];
//                     const uint32_t maskA = broadGridCell.collisionMasks.list[i];
//                     const uint32_t layerA = broadGridCell.collisionLayers.list[i];
//                     const Entity entityA = broadGridCell.entities.list[i];

//                     const __m256i aMinXVec = _mm256_set1_epi8(aMinX);
//                     const __m256i aMinYVec = _mm256_set1_epi8(aMinY);

//                     size_t j = i + 1;
//                     // for (; j + 31 < count; j += 32)
//                     // {
//                     //     __m256i bMinXVec = _mm256_loadu_si256((__m256i*)&broadGridCell.minCellX.list[j]);
//                     //     __m256i bMinYVec = _mm256_loadu_si256((__m256i*)&broadGridCell.minCellY.list[j]);

//                     //     __m256i sharedMinX = _mm256_max_epu8(aMinXVec, bMinXVec);
//                     //     __m256i sharedMinY = _mm256_max_epu8(aMinYVec, bMinYVec);

//                     //     __m256i sharedMinXLo = _mm256_unpacklo_epi8(sharedMinX, zero);
//                     //     __m256i sharedMinXHi = _mm256_unpackhi_epi8(sharedMinX, zero);
//                     //     __m256i sharedMinYLo = _mm256_unpacklo_epi8(sharedMinY, zero);
//                     //     __m256i sharedMinYHi = _mm256_unpackhi_epi8(sharedMinY, zero);

//                     //     __m256i ownerLo = _mm256_add_epi16(_mm256_mullo_epi16(sharedMinYLo, gridCols), sharedMinXLo);
//                     //     __m256i ownerHi = _mm256_add_epi16(_mm256_mullo_epi16(sharedMinYHi, gridCols), sharedMinXHi);

//                     //     __m256i matchLo = _mm256_cmpeq_epi16(ownerLo, currentCell);
//                     //     __m256i matchHi = _mm256_cmpeq_epi16(ownerHi, currentCell);

//                     //     __m256i match = _mm256_packs_epi16(matchLo, matchHi);
//                     //     uint32_t ownerMask = _mm256_movemask_epi8(match);

//                     //     while (ownerMask)
//                     //     {
//                     //         int lane = std::countr_zero(ownerMask);
//                     //         size_t index = j + lane;

//                     //         uint32_t maskB = broadGridCell.collisionMasks.list[index];
//                     //         uint32_t layerB = broadGridCell.collisionLayers.list[index];

//                     //         if (!CanCollidersContact(maskA, maskB, static_cast<Layer>(layerA), static_cast<Layer>(layerB)))
//                     //         {
//                     //             ownerMask &= ownerMask - 1;
//                     //             continue;
//                     //         }

//                     //         const uint16_t solverBodyBIndex = broadGridCell.solverBodyIndices.list[index];
//                     //         const bool aIsLower = solverBodies[solverBodyAIndex].entity.id < solverBodies[solverBodyBIndex].entity.id;
//                     //         size_t sA = aIsLower ? solverBodyAIndex : solverBodyBIndex;
//                     //         size_t sB = aIsLower ? solverBodyBIndex : solverBodyAIndex;

//                     //         m_narrowPhasebatch.aSolverBodyIndex.push_back(sA);
//                     //         m_narrowPhasebatch.bSolverBodyIndex.push_back(sB);
//                     //         m_narrowPhasebatch.aPositionX.push_back(solverBodies[sA].position.x);
//                     //         m_narrowPhasebatch.aPositionY.push_back(solverBodies[sA].position.y);
//                     //         m_narrowPhasebatch.aColliderHalfSizeX.push_back(solverBodies[sA].colliderHalfSize.x);
//                     //         m_narrowPhasebatch.aColliderHalfSizeY.push_back(solverBodies[sA].colliderHalfSize.y);
//                     //         m_narrowPhasebatch.bPositionX.push_back(solverBodies[sB].position.x);
//                     //         m_narrowPhasebatch.bPositionY.push_back(solverBodies[sB].position.y);
//                     //         m_narrowPhasebatch.bColliderHalfSizeX.push_back(solverBodies[sB].colliderHalfSize.x);
//                     //         m_narrowPhasebatch.bColliderHalfSizeY.push_back(solverBodies[sB].colliderHalfSize.y);
//                     //     }
//                     // }

//                     {
//                         ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/ForeachActiveCell3");
//                         for (; j < count; ++j)
//                         {
//                             {
//                                 ZoneScopedN("BroadPhaseSystem/FindCollisionPairs/ForeachActiveCell3/PreGather");
//                                 const uint16_t sharedMinX = std::max(aMinX, broadGridCell.minCellX.list[j]);
//                                 const uint16_t sharedMinY = std::max(aMinY, broadGridCell.minCellY.list[j]);
//                                 const uint16_t ownerCellIndex = sharedMinY * m_gridCols + sharedMinX;
//                                 if (currentCellIndex != ownerCellIndex) continue;

//                                 uint32_t maskB = broadGridCell.collisionMasks.list[j];
//                                 uint32_t layerB = broadGridCell.collisionLayers.list[j];

//                                 if (!CanCollidersContact(maskA, maskB, static_cast<Layer>(layerA), static_cast<Layer>(layerB))) continue;

//                                 const Entity entityB = broadGridCell.entities.list[j];
//                                 const uint16_t solverBodyBIndex = broadGridCell.solverBodyIndices.list[j];
//                                 const bool aIsLower = entityA.id < entityB.id;
//                                 uint16_t sA = aIsLower ? solverBodyAIndex : solverBodyBIndex;
//                                 uint16_t sB = aIsLower ? solverBodyBIndex : solverBodyAIndex;

//                                 m_solverBodyPairs.AddPair(sA, sB);
//                             }
//                         }
//                     }
//                 }
//             }
//         }
//     }
// }
