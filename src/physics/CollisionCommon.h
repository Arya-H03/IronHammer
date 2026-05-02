#pragma once

#include "core/CoreComponents.hpp"
#include "core/memory/InlineEntityList.h"
#include "core/memory/InlineVector.h"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "editor/entityInspector/componentGui/ComponentGui.h"

#include <SFML/Graphics/Color.hpp>
#include <cassert>
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
    Entity entities[size];
    uint16_t solverBodyIndices[size];
    uint8_t minCellX[size];
    uint8_t minCellY[size];
    uint32_t collisionMasks[size];
    uint32_t collisionLayers[size];

    size_t count = 0;

    void Add(Entity entity, uint16_t solverBodyIndex, Vect2<uint8_t> minCell, uint32_t collisionMask, uint32_t collisionLayer)
    {
        entities[count] = entity;
        solverBodyIndices[count] = solverBodyIndex;
        this->minCellX[count] = minCell.x;
        this->minCellY[count] = minCell.y;
        collisionMasks[count] = collisionMask;
        collisionLayers[count] = collisionLayer;
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

template <size_t size>
struct BroadGridCellCenterCell
{
    InlineVector<Entity, size> entities;
    InlineVector<uint16_t, size> solverBodyIndices;
    InlineVector<uint32_t, size> collisionMasks;
    InlineVector<uint32_t, size> collisionLayers;

    void Add(Entity entity, uint16_t solverBodyIndex, uint32_t collisionMask, uint32_t collisionLayer)
    {
        entities.Push(entity);
        solverBodyIndices.Push(solverBodyIndex);
        collisionMasks.Push(collisionMask);
        collisionLayers.Push(collisionLayer);
    }

    void Clear()
    {
        solverBodyIndices.Clear();
        entities.Clear();
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
    std::vector<uint32_t> colliderMasks;
    std::vector<uint32_t> colliderLayers;
    std::vector<CTransform*> transformPtrs;

    void AddSolverBody(Entity entity, const Vect2f& position, const Vect2f& colliderHalfSize, float inverseMass, uint32_t collisionMask,
                       uint32_t collisionLayer, CTransform* transformPtr)
    {
        entites.push_back(entity);
        posX.push_back(position.x);
        posY.push_back(position.y);
        colliderHalfSizeX.push_back(colliderHalfSize.x);
        colliderHalfSizeY.push_back(colliderHalfSize.y);
        inverseMasses.push_back(inverseMass);
        colliderMasks.push_back(collisionMask);
        colliderLayers.push_back(collisionLayer);
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
        colliderMasks.reserve(size);
        colliderLayers.reserve(size);
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
        colliderMasks.clear();
        colliderLayers.clear();
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

struct alignas(32) NarrowPhaseSIMDBatch
{
    float aPositionX[32];
    float aPositionY[32];
    float aColliderHalfSizeX[32];
    float aColliderHalfSizeY[32];

    float bPositionX[32];
    float bPositionY[32];
    float bColliderHalfSizeX[32];
    float bColliderHalfSizeY[32];

    float aMass[32];
    float bMass[32];

    uint16_t aSolverBodyIndices[32];
    uint16_t bSolverBodyIndices[32];

    size_t count = 0;
};
