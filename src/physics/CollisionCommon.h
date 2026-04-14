#pragma once

#include "core/CoreComponents.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstdint>

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

struct CollisionPairData
{
    Entity e1, e2;

    CTransform* e1TransformPtr;
    CCollider* e1ColliderPtr;
    CRigidBody* e1RigidBodyPtr;

    CTransform* e2TransformPtr;
    CCollider* e2ColliderPtr;
    CRigidBody* e2RigidBodyPtr;

    bool operator==(const CollisionPairOld& otherPair) const
    {
        return e1 == otherPair.e1 && e2 == otherPair.e2;
    }
};

struct BroadPhaseCellData
{
    size_t cellIndex = SIZE_MAX;
    Entity entity;
    bool isHome;

    CTransform* transformPtr;
    CCollider* colliderPtr;
    CRigidBody* rigidBodyPtr;
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
    Entity e1, e2;
    Vect2f normal;
    float penetration;

    CTransform* e1TransformPtr;
    CRigidBody* e1RigidBodyPtr;

    CTransform* e2TransformPtr;
    CRigidBody* e2RigidBodyPtr;
};
