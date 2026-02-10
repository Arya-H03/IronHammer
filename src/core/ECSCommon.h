#include <cstdint>
#pragma once

using ArchetypeID = uint32_t;
static constexpr ArchetypeID InvalidArchetypeID = UINT32_MAX;

using EntityID = uint32_t;
static constexpr EntityID InvalidEntityID = UINT32_MAX;

struct Entity
{
    EntityID id;
    uint32_t generation;
};

struct EntityArchetypeLocation
{
    ArchetypeID archetypeId = InvalidArchetypeID;
    uint32_t chunkIndex = UINT32_MAX;
    uint32_t indexInChunk = UINT32_MAX;

    static constexpr EntityArchetypeLocation InvalidLocation()
    {
        return {InvalidArchetypeID, UINT32_MAX, UINT32_MAX};
    }
};
