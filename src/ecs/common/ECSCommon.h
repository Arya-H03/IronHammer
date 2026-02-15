#pragma once
#include <cstdint>
#include <bitset>


static constexpr uint16_t MaxComponents = 32;
using ComponentID = uint32_t;

using ArchetypeId = uint32_t;
using ComponentSignatureMask = std::bitset<MaxComponents>;
static constexpr ArchetypeId InvalidArchetypeID = UINT32_MAX;

using EntityID = uint32_t;
static constexpr EntityID InvalidEntityID = UINT32_MAX;

struct Entity
{
    EntityID id = InvalidEntityID;
    uint32_t generation = InvalidEntityID;
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
