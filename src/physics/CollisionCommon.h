#pragma once

#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
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

struct CollisionPair
{
    Entity e1, e2;

    bool operator==(const CollisionPair& otherPair) const
    {
        return e1 == otherPair.e1 && e2 == otherPair.e2;
    }
};

struct PotentialCollisionPair
{
    std::size_t solverBodyAIndex;
    std::size_t solverBodyBIndex;
};

struct BroadPhaseCellData
{
    uint16_t solverBodyIndex;
    Vect2<uint8_t> minCell;
};

struct SolverBody
{
    Entity entity;
    Vect2f position{};
    Vect2f colliderHalfSize{};
    float inverseMass = 0;
    uint32_t collisionMask = ~0u;
    Layer collisionLayer = Layer::Default;
    CTransform* transformPtr = nullptr;
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

struct NarrowPhaseSIMDBatch
{
    std::vector<float> aPositionX, aPositionY, aColliderHalfSizeX, aColliderHalfSizeY;
    std::vector<float> bPositionX, bPositionY, bColliderHalfSizeX, bColliderHalfSizeY;

    std::vector<uint32_t> aSolverBodyIndex;
    std::vector<uint32_t> bSolverBodyIndex;

    size_t count = 0;

    NarrowPhaseSIMDBatch()
    {
        Reserve(75000);
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

        count = size;
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
