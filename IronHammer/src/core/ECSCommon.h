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
	ArchetypeID archetypeId;
	uint32_t chunkIndex;
	uint32_t indexInChunk;
};

