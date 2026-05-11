#pragma once

#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"

#include <SFML/Graphics/Color.hpp>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

struct CollisionPair
{
    Entity e1, e2;

    bool operator==(const CollisionPair& otherPair) const
    {
        return e1 == otherPair.e1 && e2 == otherPair.e2;
    }
};

struct CollisionPairHash
{
    size_t operator()(const CollisionPair& pair) const
    {
        return std::hash<uint32_t>()(pair.e1.id) ^ (std::hash<uint32_t>()(pair.e2.id) << 1);
    }
};

struct CollisionResults
{
    std::vector<float> normalX;
    std::vector<float> normalY;
    std::vector<float> penetration;

    std::vector<float> invMassA;
    std::vector<float> invMassB;

    std::vector<uint16_t> solverBodyAIndex;
    std::vector<uint16_t> solverBodyBIndex;

    size_t Count() const
    {
        return normalX.size();
    }

    void Clear()
    {
        normalX.clear();
        normalY.clear();
        penetration.clear();
        invMassA.clear();
        invMassB.clear();
        solverBodyAIndex.clear();
        solverBodyBIndex.clear();
    }

    void Reserve(size_t count)
    {
        normalX.reserve(count);
        normalY.reserve(count);
        penetration.reserve(count);
        invMassA.reserve(count);
        invMassB.reserve(count);
        solverBodyAIndex.reserve(count);
        solverBodyBIndex.reserve(count);
    }
};


template <size_t size>
struct BroadPhaseGridCell
{
    uint16_t solverBodyIndices[size];
    uint8_t minCellX[size];
    uint8_t minCellY[size];

    size_t count = 0;

    void Add(uint16_t solverBodyIndex, uint8_t minX, uint8_t minY)
    {
        solverBodyIndices[count] = solverBodyIndex;
        minCellX[count] = minX;
        minCellY[count] = minY;

        count++;
        assert(count <= size && "BroadGridCell overflow");
    }

    void Clear()
    {
        count = 0;
    }

    size_t Size() const
    {
        return count;
    }
};

struct BroadPhaseCellDataEntry
{
    uint16_t cellIndex;
    uint16_t solverBodyIndex;
    uint8_t minCellX;
    uint8_t minCellY;
};

struct alignas(64) BroadPhaseCellDataBuffer
{
    std::vector<BroadPhaseCellDataEntry> entries;

    void Clear()
    {
        entries.clear();
    }

    void Reserve(size_t size, uint16_t maxCellIndex)
    {
        entries.reserve(size);
    }
};

struct SolverBodies
{
    std::vector<float> posX;
    std::vector<float> posY;
    std::vector<float> prePosX;
    std::vector<float> prePosY;

    std::vector<float> colliderHalfSizeX;
    std::vector<float> colliderHalfSizeY;
    std::vector<float> colliderOffsetX;
    std::vector<float> colliderOffsetY;
    std::vector<uint32_t> colliderMasks;
    std::vector<uint32_t> colliderLayers;

    std::vector<float> inverseMasses;
    std::vector<Entity> entites;
    std::vector<CTransform*> transformPtrs;

    void AddSolverBody(Entity entity, const Vect2f& position, const Vect2f& prePosition, const Vect2f& colliderOffset,
                       const Vect2f& colliderHalfSize, float inverseMass, uint32_t collisionMask, uint32_t collisionLayer,
                       CTransform* transformPtr)
    {
        entites.push_back(entity);

        posX.push_back(position.x);
        posY.push_back(position.y);

        prePosX.push_back(prePosition.x);
        prePosY.push_back(prePosition.y);

        colliderHalfSizeX.push_back(colliderHalfSize.x);
        colliderHalfSizeY.push_back(colliderHalfSize.y);
        colliderOffsetX.push_back(colliderOffset.x);
        colliderOffsetY.push_back(colliderOffset.y);
        colliderMasks.push_back(collisionMask);
        colliderLayers.push_back(collisionLayer);

        inverseMasses.push_back(inverseMass);
        transformPtrs.push_back(transformPtr);
    }

    void Reserve(size_t size)
    {
        entites.reserve(size);

        posX.reserve(size);
        posY.reserve(size);
        prePosX.reserve(size);
        prePosY.reserve(size);

        colliderOffsetX.reserve(size);
        colliderOffsetY.reserve(size);
        colliderHalfSizeX.reserve(size);
        colliderHalfSizeY.reserve(size);
        colliderMasks.reserve(size);
        colliderLayers.reserve(size);

        inverseMasses.reserve(size);
        transformPtrs.reserve(size);
    }

    void Clear()
    {
        entites.clear();

        posX.clear();
        posY.clear();
        prePosX.clear();
        prePosY.clear();

        colliderHalfSizeX.clear();
        colliderHalfSizeY.clear();
        colliderOffsetX.clear();
        colliderOffsetY.clear();
        colliderMasks.clear();
        colliderLayers.clear();

        inverseMasses.clear();
        transformPtrs.clear();
    }

    size_t Count() const
    {
        return entites.size();
    }
};

struct SolverBodyPairs
{
    std::vector<uint16_t> bodyAIndices;
    std::vector<uint16_t> bodyBIndices;

    size_t Count() const
    {
        return bodyAIndices.size();
    }

    void AddPair(uint16_t bodyAIndex, uint16_t bodyBIndex)
    {
        bodyAIndices.push_back(bodyAIndex);
        bodyBIndices.push_back(bodyBIndex);
    }

    void Clear()
    {
        bodyAIndices.clear();
        bodyBIndices.clear();
    }

    void Reserve(size_t size)
    {
        bodyAIndices.reserve(size);
        bodyBIndices.reserve(size);
    }

    void Resize(size_t size)
    {
        bodyAIndices.resize(size);
        bodyBIndices.resize(size);
    }
};

struct alignas(64) BroadPhaseBodyPairBuffer
{
    std::vector<uint16_t> bodyAIndices;
    std::vector<uint16_t> bodyBIndices;

    size_t Count() const
    {
        return bodyAIndices.size();
    }

    void AddPair(uint16_t bodyAIndex, uint16_t bodyBIndex)
    {
        bodyAIndices.push_back(bodyAIndex);
        bodyBIndices.push_back(bodyBIndex);
    }

    void Clear()
    {
        bodyAIndices.clear();
        bodyBIndices.clear();
    }

    void Reserve(size_t size)
    {
        bodyAIndices.reserve(size);
        bodyBIndices.reserve(size);
    }
};
