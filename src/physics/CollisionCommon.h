#pragma once

#include "ecs/common/ECSCommon.h"
#include "core/utils/Vect2.hpp"

#include <cmath>
#include <cstdint>
#include <SFML/Graphics/Color.hpp>

struct CollisionPair
{
    Entity e1, e2;

    CollisionPair(Entity a, Entity b)
    {
        e1 = a.id < b.id ? a : b;
        e2 = b.id > a.id ? b : a;
    }
    bool operator==(const CollisionPair& otherPair) const { return e1 == otherPair.e1 && e2 == otherPair.e2; }
};

struct CollisionPairHash
{
    size_t operator()(const CollisionPair& pair) const
    {
        return std::hash<uint32_t>()(pair.e1.id) ^ (std::hash<uint32_t>()(pair.e2.id) << 1);
    }
};

struct CollisionData
{
    Entity e1, e2;
    Vect2f normal;
    float  penetration;
};
