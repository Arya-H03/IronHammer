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
#include <SFML/Window/Keyboard.hpp>
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
    m_threadCount = threadPool.ThreadCount() + 1;
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_gridRows = (Viewport::GetSize().y / m_cellSize) + 5;
    m_gridCols = (Viewport::GetSize().x / m_cellSize) + 5;
    m_cellCount = m_gridCols * m_gridRows;

    m_threadPages.resize(m_threadCount);
    for (auto& threadPage : m_threadPages)
    {
        threadPage.resize(m_cellCount);
    }
    m_globalCellOffsets.resize(m_cellCount);

    m_cellDataBuffer.resize(m_threadCount);
    for (auto& buffer : m_cellDataBuffer)
    {
        buffer.Reserve(20000, m_cellCount);
    }

    m_bodyPairBuffer.resize(m_threadCount);
    for (auto& buffer : m_bodyPairBuffer)
    {
        buffer.Reserve(20000);
    }

    m_coutingCells.reserve(m_cellCount);

    m_broadPhaseQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
}

void BroadPhaseCollisionSystem::GatherCellDataIntoBuffer(size_t threadIndex)
{
    BroadPhaseCellDataBuffer& threadBuffer = m_cellDataBuffer[threadIndex];
    threadBuffer.Clear();

    std::vector<uint16_t>& threadPage = m_threadPages[threadIndex];
    std::fill(threadPage.begin(), threadPage.end(), 0);

    const size_t totalSolverBodyCount = m_solverBodies.Count();
    const size_t tail = totalSolverBodyCount % m_threadCount;
    const size_t chunkSize = (totalSolverBodyCount - tail) / m_threadCount;
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
                uint16_t cellIndex = (uint16_t)(y * m_gridCols + x);
                threadBuffer.entries.push_back({cellIndex, i, (uint8_t)xStart, (uint8_t)yStart});
                ++threadPage[cellIndex];
            }
        }
    }
}
void BroadPhaseCollisionSystem::ComputeCellGlobalOffset()
{
    uint16_t offset = 0;
    for (size_t cellIndex = 0; cellIndex < m_cellCount; ++cellIndex)
    {
        m_globalCellOffsets[cellIndex] = offset;

        for (size_t threadIndex = 0; threadIndex < m_threadCount; ++threadIndex)
        {
            uint16_t cellCount = m_threadPages[threadIndex][cellIndex];
            m_threadPages[threadIndex][cellIndex] = offset;
            offset += cellCount;
        }
    }
    m_mergedCellData.resize(offset);
}
void BroadPhaseCollisionSystem::ScatterCellData(size_t threadIndex)
{
    std::vector<uint16_t>& threadPage = m_threadPages[threadIndex];
    for (const auto& entry : m_cellDataBuffer[threadIndex].entries)
    {
        m_mergedCellData[threadPage[entry.cellIndex]++] = entry;
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

    {
        // ZoneScopedN("CollisionBroadPhase/SortCellData/Setup");
        m_tempCellData.resize(m_mergedCellData.size());
        m_coutingCells.assign(cellCount, 0);
    }

    {
        // ZoneScopedN("CollisionBroadPhase/SortCellData/CellDataLoop");
        for (const auto& entry : m_mergedCellData)
        {
            ++m_coutingCells[entry.cellIndex];
        }
    }

    {
        // ZoneScopedN("CollisionBroadPhase/SortCellData/CoutingCellLoop");
        for (size_t i = 1; i < cellCount; ++i)
        {
            m_coutingCells[i] += m_coutingCells[i - 1];
        }
    }

    {
        // ZoneScopedN("CollisionBroadPhase/SortCellData/SortLoop");
        for (int i = (int)m_mergedCellData.size() - 1; i >= 0; --i)
        {
            m_tempCellData[--m_coutingCells[m_mergedCellData[i].cellIndex]] = m_mergedCellData[i];
        }
    }

    {
        // ZoneScopedN("CollisionBroadPhase/SortCellData/Swap");
        std::swap(m_mergedCellData, m_tempCellData);
    }
}

void BroadPhaseCollisionSystem::FindThreadWorkloadBoundsThreadedSort()
{
    const size_t totalEntries = m_mergedCellData.size();
    const size_t minThreadLoad = totalEntries / m_threadCount;

    m_threadIndexBounds.clear();
    size_t entryStart = 0;

    for (size_t i = 0; i < m_cellCount; ++i)
    {
        size_t nextCellStart = (i + 1 < m_cellCount) ? m_globalCellOffsets[i + 1] : totalEntries;

        if (nextCellStart - entryStart >= minThreadLoad)
        {
            m_threadIndexBounds.push_back({entryStart, nextCellStart - 1});
            entryStart = nextCellStart;
        }
    }

    if (entryStart < totalEntries)
    {
        if (m_threadIndexBounds.size() == m_threadCount)
        {
            m_threadIndexBounds.back().second = totalEntries - 1;
        }
        else
        {
            m_threadIndexBounds.push_back({entryStart, totalEntries - 1});
        }
    }
}

void BroadPhaseCollisionSystem::FindThreadWorkloadBoundsRegSort()
{
    const size_t totalEntries = m_mergedCellData.size();
    const size_t minThreadLoad = totalEntries / m_threadCount;

    m_threadIndexBounds.clear();

    size_t boundsStartIndex = 0;
    size_t boundsLength = 0;

    for (size_t i = 0; i < m_cellCount; ++i)
    {
        size_t nextStartIndex = (i + 1 < m_cellCount) ? m_coutingCells[i + 1] : m_mergedCellData.size();
        size_t currentBoundsLength = nextStartIndex - m_coutingCells[i];
        boundsLength += currentBoundsLength;

        if (boundsLength >= minThreadLoad || i == m_cellCount - 1)
        {
            m_threadIndexBounds.push_back({boundsStartIndex, boundsStartIndex + boundsLength - 1});
            boundsStartIndex += boundsLength;
            boundsLength = 0;
        }
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
        // ZoneScopedN("CollisionBroadPhase/GatherCellDataIntoBuffer(Threaded)");
        m_threadPool.Dispatch(m_threadCount, [this](size_t threadIndex) { GatherCellDataIntoBuffer(threadIndex); });
    }

    // {
    //     // ZoneScopedN("CollisionBroadPhase/MergeCellDataBuffers");
    //     MergeCellDataBuffers();
    // }
    // {
    //     // ZoneScopedN("CollisionBroadPhase/SortCellData");
    //     SortCellData();
    // }
    // {
    //     // ZoneScopedN("CollisionBroadPhase/FindThreadWorkloadBoundsRegSort");
    //     FindThreadWorkloadBoundsRegSort();
    // }

    {
        // ZoneScopedN("CollisionBroadPhase/ComputeGlobalOffsets");
        ComputeCellGlobalOffset();
    }
    {
        // ZoneScopedN("CollisionBroadPhase/ScatterCellData(Threaded)");
        m_threadPool.Dispatch(m_threadCount, [this](size_t threadIndex) { ScatterCellData(threadIndex); });
    }
    {
        // ZoneScopedN("CollisionBroadPhase/FindThreadWorkloadBoundsThreadedSort");
        FindThreadWorkloadBoundsThreadedSort();
    }


    {
        // ZoneScopedN("CollisionBroadPhase/FindCollisionPairs(Threaded)");
        m_threadPool.Dispatch(m_threadCount, [this](size_t threadIndex) { GatherCollisionPairsIntoBuffer(threadIndex); });
    }

    {
        // ZoneScopedN("CollisionBroadPhase/MergeCollisionPairs");
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
