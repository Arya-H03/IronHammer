#pragma once

#include "core/CoreComponents.hpp"
#include "core/memory/InlineVector.h"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"

#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

struct Cell
{
    Vect2<int> coord;
    Vect2f pos;
    std::vector<Entity> overlapingEntities;

    Cell() = default;
    Cell(Vect2<int> c, Vect2f p) : coord(c), pos(p)
    {
    }
};
struct CollisionPair
{
    Entity e1, e2;

    bool operator==(const CollisionPair& otherPair) const
    {
        return e1 == otherPair.e1 && e2 == otherPair.e2;
    }
};
struct CollisionPairOld
{
    Entity e1, e2;

    CollisionPairOld(Entity a, Entity b)
    {
        e1 = a.id < b.id ? a : b;
        e2 = a.id < b.id ? b : a;
    }

    bool operator==(const CollisionPairOld& otherPair) const
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

struct CollisionCorrectionData
{
    size_t solverBodyAIndex;
    size_t solverBodyBIndex;

    Vect2f normal;
    float penetration;
};


template <size_t size>
struct BroadGridCell
{
    InlineVector<Entity, size> entities;
    InlineVector<uint16_t, size> solverBodyIndices;
    InlineVector<uint8_t, size> minCellX;
    InlineVector<uint8_t, size> minCellY;
    InlineVector<uint32_t, size> collisionMasks;
    InlineVector<uint32_t, size> collisionLayers;

    void Add(Entity entity, uint16_t solverBodyIndex, Vect2<uint8_t> minCell, uint32_t collisionMask, uint32_t collisionLayer)
    {
        entities.Push(entity);
        solverBodyIndices.Push(solverBodyIndex);
        this->minCellX.Push(minCell.x);
        this->minCellY.Push(minCell.y);
        collisionMasks.Push(collisionMask);
        collisionLayers.Push(collisionLayer);
    }

    void Clear()
    {
        solverBodyIndices.Clear();
        entities.Clear();
        minCellX.Clear();
        minCellY.Clear();
        collisionMasks.Clear();
        collisionLayers.Clear();
    }

    size_t Size() const
    {
        return solverBodyIndices.Size();
    }
};

struct SolverBodies
{
    std::vector<Entity> entites;
    std::vector<float> posX;
    std::vector<float> posY;
    std::vector<float> colliderHalfSizeX;
    std::vector<float> colliderHalfSizeY;
    std::vector<float> inverseMasses;
    std::vector<CTransform*> transformPtrs;

    void AddSolverBody(Entity entity, const Vect2f& position, const Vect2f& colliderHalfSize, float inverseMass, CTransform* transformPtr)
    {
        entites.push_back(entity);
        posX.push_back(position.x);
        posY.push_back(position.y);
        colliderHalfSizeX.push_back(colliderHalfSize.x);
        colliderHalfSizeY.push_back(colliderHalfSize.y);
        inverseMasses.push_back(inverseMass);
        transformPtrs.push_back(transformPtr);
    }

    void Reserve(size_t size)
    {
        entites.reserve(size);
        posX.reserve(size);
        posY.reserve(size);
        colliderHalfSizeX.reserve(size);
        colliderHalfSizeY.reserve(size);
        inverseMasses.reserve(size);
        transformPtrs.reserve(size);
    }

    void Clear()
    {
        entites.clear();
        posX.clear();
        posY.clear();
        colliderHalfSizeX.clear();
        colliderHalfSizeY.clear();
        inverseMasses.clear();
        transformPtrs.clear();
    }

    size_t Count()
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
};

struct NarrowPhaseSIMDBatch
{
    std::vector<float> aPositionX, aPositionY, aColliderHalfSizeX, aColliderHalfSizeY;
    std::vector<float> bPositionX, bPositionY, bColliderHalfSizeX, bColliderHalfSizeY;

    std::vector<uint32_t> aSolverBodyIndex;
    std::vector<uint32_t> bSolverBodyIndex;

    void Add(uint32_t aIndex, uint32_t bIndex, float aPosX, float aPosY, float aHalfSizeX, float aHalfSizeY, float bPosX, float bPosY,
             float bHalfSizeX, float bHalfSizeY)
    {
        aSolverBodyIndex.push_back(aIndex);
        bSolverBodyIndex.push_back(bIndex);
        aPositionX.push_back(aPosX);
        aPositionY.push_back(aPosY);
        aColliderHalfSizeX.push_back(aHalfSizeX);
        aColliderHalfSizeY.push_back(aHalfSizeY);
        bPositionX.push_back(bPosX);
        bPositionY.push_back(bPosY);
        bColliderHalfSizeX.push_back(bHalfSizeX);
        bColliderHalfSizeY.push_back(bHalfSizeY);
    }

    NarrowPhaseSIMDBatch()
    {
        Reserve(100000);
    }

    size_t Count() const
    {
        return aSolverBodyIndex.size();
    }

    void Reserve(size_t size)
    {
        aPositionX.reserve(size);
        aPositionY.reserve(size);
        aColliderHalfSizeX.reserve(size);
        aColliderHalfSizeY.reserve(size);

        bPositionX.reserve(size);
        bPositionY.reserve(size);
        bColliderHalfSizeX.reserve(size);
        bColliderHalfSizeY.reserve(size);

        aSolverBodyIndex.reserve(size);
        bSolverBodyIndex.reserve(size);
    }

    void Resize(size_t size)
    {
        aPositionX.resize(size);
        aPositionY.resize(size);
        aColliderHalfSizeX.resize(size);
        aColliderHalfSizeY.resize(size);

        bPositionX.resize(size);
        bPositionY.resize(size);
        bColliderHalfSizeX.resize(size);
        bColliderHalfSizeY.resize(size);

        aSolverBodyIndex.resize(size);
        bSolverBodyIndex.resize(size);
    }

    void Clear()
    {
        aPositionX.clear();
        aPositionY.clear();
        aColliderHalfSizeX.clear();
        aColliderHalfSizeY.clear();

        bPositionX.clear();
        bPositionY.clear();
        bColliderHalfSizeX.clear();
        bColliderHalfSizeY.clear();

        aSolverBodyIndex.clear();
        bSolverBodyIndex.clear();
    }
};
