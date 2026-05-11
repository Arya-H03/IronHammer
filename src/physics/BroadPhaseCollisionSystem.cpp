#include "BroadPhaseCollisionSystem.h"

#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Threadpool.h"
#include "core/utils/Vect2.hpp"
#include "editor/Viewport.h"
#include "physics/CollisionCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/Joystick.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <emmintrin.h>
#include <immintrin.h>
#include <sys/stat.h>
#include <utility>
#include <vector>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(SolverBodies& solverBodies, ThreadPool& threadPool)
    : m_solverBodies(solverBodies), m_threadPool(threadPool)
{
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_gridRows = (Viewport::GetSize().y / m_cellSize) + 5;
    m_gridCols = (Viewport::GetSize().x / m_cellSize) + 5;

    m_cellDataBuffer.resize(m_threadPool.ThreadCount() + 1);
    for (auto& buffer : m_cellDataBuffer)
    {
        buffer.Reserve(20000, m_gridCols * m_gridRows);
    }

    m_bodyPairBuffer.resize(m_threadPool.ThreadCount() + 1);
    for (auto& buffer : m_bodyPairBuffer)
    {
        buffer.Reserve(20000);
    }

    m_coutingCells.reserve(m_gridCols * m_gridRows);

    m_broadPhaseQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
}

void BroadPhaseCollisionSystem::GatherCellDataIntoBuffer(size_t threadIndex)
{
    BroadPhaseCellDataBuffer& threadBuffer = m_cellDataBuffer[threadIndex];
    threadBuffer.Clear();

    const size_t totalSolverBodyCount = m_solverBodies.Count();
    const size_t totalThreadCount = m_threadPool.ThreadCount() + 1;
    const size_t tail = totalSolverBodyCount % totalThreadCount;
    const size_t chunkSize = (totalSolverBodyCount - tail) / totalThreadCount;
    const size_t startIndex = chunkSize * threadIndex;
    const size_t endIndex = threadIndex != m_threadPool.ThreadCount() ? startIndex + chunkSize : totalSolverBodyCount;

    for (uint16_t i = startIndex; i < endIndex; ++i)
    {
        Vect2f center{m_solverBodies.posX[i], m_solverBodies.posY[i]};
        Vect2f colliderHalfSize{m_solverBodies.colliderHalfSizeX[i], m_solverBodies.colliderHalfSizeY[i]};
        Vect2<uint8_t> topLeftCell = ((center - (colliderHalfSize)) * m_cellSizeInv).FloorCast<uint8_t>();
        Vect2<uint8_t> bottomRightCell = ((center + (colliderHalfSize)) * m_cellSizeInv).FloorCast<uint8_t>();

        int yStart = std::max((int)topLeftCell.y, 0);
        int yEnd = std::min((int)bottomRightCell.y, (int)m_gridRows - 1);
        int xStart = std::max((int)topLeftCell.x, 0);
        int xEnd = std::min((int)bottomRightCell.x, (int)m_gridCols - 1);

        for (int y = yStart; y <= yEnd; ++y)
        {
            for (int x = xStart; x <= xEnd; ++x)
            {
                threadBuffer.entries.push_back({(uint16_t)(y * m_gridCols + x), i, (uint8_t)xStart, (uint8_t)yStart});
            }
        }
    }
}

void BroadPhaseCollisionSystem::MergeCellDataBuffers()
{
    size_t totalCount = 0;
    for (auto& broadPhaseThreadBuffer : m_cellDataBuffer)
    {
        totalCount += broadPhaseThreadBuffer.entries.size();
    }
    m_mergedCellData.resize(totalCount);

    size_t currentCount = 0;
    for (auto& broadPhaseThreadBuffer : m_cellDataBuffer)
    {
        size_t bufferSize = broadPhaseThreadBuffer.entries.size();
        std::memcpy(m_mergedCellData.data() + currentCount, broadPhaseThreadBuffer.entries.data(),
                    bufferSize * sizeof(BroadPhaseCellDataEntry));
        currentCount += bufferSize;
    }
}

void BroadPhaseCollisionSystem::SortCellData()
{
    const size_t cellCount = m_gridCols * m_gridRows;
    m_tempCellData.resize(m_mergedCellData.size());
    m_coutingCells.assign(cellCount, 0);

    for (const auto& entry : m_mergedCellData)
    {
        ++m_coutingCells[entry.cellIndex];
    }

    for (size_t i = 1; i < cellCount; ++i)
    {
        m_coutingCells[i] += m_coutingCells[i - 1];
    }

    for (int i = (int)m_mergedCellData.size() - 1; i >= 0; --i)
    {
        m_tempCellData[--m_coutingCells[m_mergedCellData[i].cellIndex]] = m_mergedCellData[i];
    }

    std::swap(m_mergedCellData, m_tempCellData);
}

void BroadPhaseCollisionSystem::FindThreadWorkloadBounds()
{
    const size_t minThreadLoad = m_mergedCellData.size() / (m_threadPool.ThreadCount() + 1);
    m_threadIndexBounds.clear();
    size_t left = 0;
    while (left < m_mergedCellData.size())
    {
        size_t right = left + 1;
        while (right + 1 < m_mergedCellData.size() &&
               (right - left < minThreadLoad - 1 || m_mergedCellData[right].cellIndex == m_mergedCellData[right + 1].cellIndex))
        {
            ++right;
        }
        m_threadIndexBounds.push_back({left, right});
        left = right + 1;
    }
}

void BroadPhaseCollisionSystem::GatherCollisionPairsIntoBuffer(size_t threadIndex)
{
    BroadPhaseBodyPairBuffer& bodyPairBuffer = m_bodyPairBuffer[threadIndex];
    bodyPairBuffer.Clear();

    const std::pair<size_t, size_t>& threadIndexBound = m_threadIndexBounds[threadIndex];
    const size_t threadStartIndex = threadIndexBound.first;
    const size_t threadEndIndex = threadIndexBound.second;
    size_t start = threadStartIndex;

    while (start <= threadEndIndex)
    {
        size_t end = start + 1;
        while (end <= threadEndIndex && m_mergedCellData[end].cellIndex == m_mergedCellData[start].cellIndex)
        {
            ++end;
        }

        for (size_t i = start; i < end; ++i)
        {
            const BroadPhaseCellDataEntry& entryA = m_mergedCellData[i];
            const uint16_t solverBodyAIndex = m_mergedCellData[i].solverBodyIndex;
            Vect2<uint8_t> minCellA = {entryA.minCellX, entryA.minCellY};

            for (size_t j = i + 1; j < end; ++j)
            {
                const uint16_t solverBodyBIndex = m_mergedCellData[j].solverBodyIndex;
                Vect2<uint8_t> minCellB = {m_mergedCellData[j].minCellX, m_mergedCellData[j].minCellY};

                const uint16_t sharedMinX = std::max(minCellA.x, minCellB.x);
                const uint16_t sharedMinY = std::max(minCellA.y, minCellB.y);
                const uint16_t ownerCellIndex = sharedMinY * m_gridCols + sharedMinX;
                if (entryA.cellIndex != ownerCellIndex) continue;

                const bool aIsLower = solverBodyAIndex < solverBodyBIndex;
                uint16_t sA = aIsLower ? solverBodyAIndex : solverBodyBIndex;
                uint16_t sB = aIsLower ? solverBodyBIndex : solverBodyAIndex;

                bodyPairBuffer.AddPair(sA, sB);
            }
        }
        start = end;
    }
}

void BroadPhaseCollisionSystem::MergeCollisionPairBuffers()
{
    size_t totalCount = 0;
    for (const auto& bodyPairBuffer : m_bodyPairBuffer)
    {
        totalCount += bodyPairBuffer.Count();
    }
    m_solverBodyPairs.Resize(totalCount);

    size_t currentCount = 0;
    for (const auto& bodyPairBuffer : m_bodyPairBuffer)
    {
        size_t bufferSize = bodyPairBuffer.Count();
        std::memcpy(m_solverBodyPairs.bodyAIndices.data() + currentCount, bodyPairBuffer.bodyAIndices.data(),
                    bufferSize * sizeof(uint16_t));
        std::memcpy(m_solverBodyPairs.bodyBIndices.data() + currentCount, bodyPairBuffer.bodyBIndices.data(),
                    bufferSize * sizeof(uint16_t));
        currentCount += bufferSize;
    }
}

SolverBodyPairs& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr)
{
    ZoneScopedN("CollisionBroadPhase");

    {
        ZoneScopedN("CollisionBroadPhase/GatherCellDataIntoBuffer(Threaded)");
        m_threadPool.Dispatch(m_threadPool.ThreadCount() + 1, [this](size_t threadIndex) { GatherCellDataIntoBuffer(threadIndex); });
    }
    {
        ZoneScopedN("CollisionBroadPhase/MergeCellDataBuffers");
        MergeCellDataBuffers();
    }
    {
        ZoneScopedN("CollisionBroadPhase/SortCellData");
        SortCellData();
    }
    {
        ZoneScopedN("CollisionBroadPhase/FindThreadWorkloadBounds");
        FindThreadWorkloadBounds();
    }

    {
        ZoneScopedN("CollisionBroadPhase/FindCollisionPairs(Threaded)");
        m_threadPool.Dispatch(m_threadPool.ThreadCount() + 1, [this](size_t threadIndex) { GatherCollisionPairsIntoBuffer(threadIndex); });
    }

    {
        ZoneScopedN("CollisionBroadPhase/MergeCollisionPairs");
        MergeCollisionPairBuffers();
    }

    return m_solverBodyPairs;
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
