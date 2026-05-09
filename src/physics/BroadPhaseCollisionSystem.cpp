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
#include <emmintrin.h>
#include <immintrin.h>
#include <vector>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(SolverBodies& solverBodies, ThreadPool& threadPool)
    : m_solverBodies(solverBodies), m_threadPool(threadPool)
{
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_gridRows = (Viewport::GetSize().y / m_cellSize) + 5;
    m_gridCols = (Viewport::GetSize().x / m_cellSize) + 5;

    m_broadPhaseGrid.resize(m_gridRows * m_gridCols);
    m_activeCellIndices.reserve(m_gridCols * m_gridRows);

    m_cellDataBuffer.resize(m_threadPool.ThreadCount() + 1);
    for (auto& buffer : m_cellDataBuffer)
    {
        buffer.Reserve(20000, m_gridCols * m_gridRows);
    }

    m_solverBodyPairs.Reserve(100000);
    m_bodyPairBuffer.resize(m_threadPool.ThreadCount() + 1);
    for (auto& buffer : m_bodyPairBuffer)
    {
        buffer.Reserve(20000);
    }

    m_broadPhaseQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
}

void BroadPhaseCollisionSystem::FillCellsWithEntityOverlaps(World* worldPtr)
{
    for (uint16_t index : m_activeCellIndices)
    {
        m_broadPhaseGrid[index].Clear();
    }

    m_activeCellIndices.clear();

    for (uint16_t k = 0; k < m_solverBodies.Count(); ++k)
    {
        Vect2f center{m_solverBodies.posX[k], m_solverBodies.posY[k]};
        Vect2f colliderHalfSize{m_solverBodies.colliderHalfSizeX[k], m_solverBodies.colliderHalfSizeY[k]};
        Vect2<uint8_t> topLeftCell = ((center - (colliderHalfSize)) / m_cellSize).FloorCast<uint8_t>();
        Vect2<uint8_t> bottomRightCell = ((center + (colliderHalfSize)) / m_cellSize).FloorCast<uint8_t>();

        int yStart = std::max((int)topLeftCell.y, 0);
        int yEnd = std::min((int)bottomRightCell.y, (int)m_gridRows - 1);
        int xStart = std::max((int)topLeftCell.x, 0);
        int xEnd = std::min((int)bottomRightCell.x, (int)m_gridCols - 1);

        for (int y = yStart; y <= yEnd; ++y)
        {
            for (int x = xStart; x <= xEnd; ++x)
            {
                uint16_t index = y * m_gridCols + x;
                auto& broadGridCell = m_broadPhaseGrid[index];
                if (broadGridCell.Size() == 0)
                {
                    m_activeCellIndices.push_back(index);
                }

                broadGridCell.Add(k, (uint8_t)xStart, (uint8_t)yStart);
            }
        }
    }
}

void BroadPhaseCollisionSystem::FillCellWithThread(size_t threadIndex)
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

    // threadBuffer.Sort(m_gridCols * m_gridRows - 1);
}

void BroadPhaseCollisionSystem::MergeCellDataBuffers()
{

    for (uint16_t index : m_activeCellIndices)
    {
        m_broadPhaseGrid[index].Clear();
    }
    m_activeCellIndices.clear();

    for (const auto& broadPhaseThreadBuffer : m_cellDataBuffer)
    {
        for (const auto& entry : broadPhaseThreadBuffer.entries)
        {
            auto& broadPhaseGridCell = m_broadPhaseGrid[entry.cellIndex];
            if (broadPhaseGridCell.Size() == 0)
            {
                m_activeCellIndices.push_back(entry.cellIndex);
            }
            broadPhaseGridCell.Add(entry.solverBodyIndex, entry.minCellX, entry.minCellY);
        }
    }
}

void BroadPhaseCollisionSystem::FindCollisionPairsThread(size_t threadIndex)
{
    BroadPhaseBodyPairBuffer& bodyPairBuffer = m_bodyPairBuffer[threadIndex];
    bodyPairBuffer.Clear();

    const size_t totalActiveCellCount = m_activeCellIndices.size();
    const size_t totalThreadCount = m_threadPool.ThreadCount() + 1;
    const size_t tail = totalActiveCellCount % totalThreadCount;
    const size_t chunkSize = (totalActiveCellCount - tail) / totalThreadCount;
    const size_t startIndex = chunkSize * threadIndex;
    const size_t endIndex = threadIndex != m_threadPool.ThreadCount() ? startIndex + chunkSize : totalActiveCellCount;

    for (size_t k = startIndex; k < endIndex; ++k)
    {
        size_t currentCellIndex = m_activeCellIndices[k];
        auto& broadGridCell = m_broadPhaseGrid[currentCellIndex];
        const size_t count = broadGridCell.Size();
        if (count < 2) continue;

        for (size_t i = 0; i < count; ++i)
        {
            const uint16_t solverBodyAIndex = broadGridCell.solverBodyIndices[i];
            Vect2<uint8_t> minCellA = {broadGridCell.minCellX[i], broadGridCell.minCellY[i]};

            for (size_t j = i + 1; j < count; ++j)
            {
                const uint16_t solverBodyBIndex = broadGridCell.solverBodyIndices[j];
                Vect2<uint8_t> minCellB = {broadGridCell.minCellX[j], broadGridCell.minCellY[j]};

                const uint16_t sharedMinX = std::max(minCellA.x, minCellB.x);
                const uint16_t sharedMinY = std::max(minCellA.y, minCellB.y);
                const uint16_t ownerCellIndex = sharedMinY * m_gridCols + sharedMinX;
                if (currentCellIndex != ownerCellIndex) continue;

                const bool aIsLower = solverBodyAIndex < solverBodyBIndex;
                uint16_t sA = aIsLower ? solverBodyAIndex : solverBodyBIndex;
                uint16_t sB = aIsLower ? solverBodyBIndex : solverBodyAIndex;

                bodyPairBuffer.AddPair(sA, sB);
            }
        }
    }
}

void BroadPhaseCollisionSystem::MergeBodyPairBuffers()
{
    {
        for (const auto& bodyPairBuffer : m_bodyPairBuffer)
        {
            for (size_t i = 0; i < bodyPairBuffer.Count(); ++i)
            {
                m_solverBodyPairs.AddPair(bodyPairBuffer.bodyAIndices[i], bodyPairBuffer.bodyBIndices[i]);
            }
        }
    }
}

SolverBodyPairs& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr)
{
    ZoneScopedN("CollisionBroadPhase");


    {
        // ZoneScopedN("CollisionBroadPhase/FillCellsWithEntityOverlaps");
        {
            ZoneScopedN("CollisionBroadPhase/FillCellsWithEntityOverlaps/ThreadDispatch");
            m_threadPool.Dispatch(m_threadPool.ThreadCount() + 1, [this](size_t threadIndex) { FillCellWithThread(threadIndex); });
        }
        {
            ZoneScopedN("CollisionBroadPhase/FillCellsWithEntityOverlaps/MergeCellDataBuffers");
            MergeCellDataBuffers();
        }
        // FillCellsWithEntityOverlaps(worldPtr);
    }

    {
        ZoneScopedN("CollisionBroadPhase/FindCollisionPairsThread");
        m_solverBodyPairs.Clear();
        m_threadPool.Dispatch(m_threadPool.ThreadCount() + 1, [this](size_t threadIndex) { FindCollisionPairsThread(threadIndex); });
        MergeBodyPairBuffers();
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
