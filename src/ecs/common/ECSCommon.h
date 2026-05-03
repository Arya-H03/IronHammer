#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <bitset>
#include <cstdint>

static constexpr uint16_t MaxComponents = 32;
using ComponentId = uint32_t;

using ArchetypeId = uint32_t;
using ComponentSignatureMask = std::bitset<MaxComponents>;
static constexpr ArchetypeId InvalidArchetypeID = UINT32_MAX;

using EntityId = uint32_t;
static constexpr EntityId InvalidEntityID = UINT32_MAX;

struct Entity
{
    EntityId id = InvalidEntityID;
    uint32_t generation = InvalidEntityID;

    bool operator==(const Entity otherEntity) const
    {
        return id == otherEntity.id && generation == otherEntity.generation;
    }
};

struct EntityStorageLocation
{
    ArchetypeId archetypeId = InvalidArchetypeID;
    uint32_t chunkIndex = UINT32_MAX;
    uint32_t indexInChunk = UINT32_MAX;

    static constexpr EntityStorageLocation InvalidLocation()
    {
        return {InvalidArchetypeID, UINT32_MAX, UINT32_MAX};
    }
};
