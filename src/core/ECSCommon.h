#include <cstdint>
#pragma once

using ArchetypeID = uint32_t;
static constexpr ArchetypeID InvalidArchetypeID = UINT32_MAX;

struct Entity
{
    uint32_t id;
    uint32_t generation;
};

struct EntityLocation
{
    ArchetypeID archetypeId = InvalidArchetypeID;
    uint32_t chunkIndex = UINT32_MAX;
    uint32_t indexInChunk = UINT32_MAX;

    static constexpr EntityLocation InvalidLocation()
    {
        return {InvalidArchetypeID, UINT32_MAX, UINT32_MAX};
    }
};
