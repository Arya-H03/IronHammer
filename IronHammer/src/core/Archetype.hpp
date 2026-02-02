#include <tuple>
#include <vector>
#include "ECSCommon.h"
#include "SlabAllocator.hpp"
#pragma once

//Address having new enitites being added that the back chunk always.

struct BaseArchetype
{
	ArchetypeID id = InvalidArchetypeID;
	virtual ~BaseArchetype() = default;
};

template<size_t ChunkSize, typename... Components>
class Archetype final : public BaseArchetype
{
	static_assert(ChunkSize > 0);
private:

	using AllocatorTuple = std::tuple<SlabAllocator<Components, ChunkSize>...>;
	AllocatorTuple allocatorTuple;

	struct ArchetypeChunk
	{
		std::tuple<Components*...> ptrToComponentArrayTuple;
		size_t count = 0;

		bool IsFull()const { return count == ChunkSize; }
	};

	std::vector<ArchetypeChunk> archetypeChunks;

	ArchetypeChunk& CreateArchetypeChunk()
	{
		ArchetypeChunk chunk;

		chunk.ptrToComponentArrayTuple = std::make_tuple(
			std::get<SlabAllocator<Components, ChunkSize>>(allocatorTuple).AllocateArray()...
		);

		archetypeChunks.push_back(chunk);
		return archetypeChunks.back();
	}

public:

	//Add Entity
	template<typename... Args>
	void AddEntity(Args&&... componentValues, EntityLocation& entityLocation)
	{
		static_assert(sizeof...(Args) == sizeof...(componentValues));

		if (archetypeChunks.empty() || archetypeChunks.back().IsFull()) CreateArchetypeChunk();

		ArchetypeChunk& chunk = archetypeChunks.back();
		const size_t index = chunk.count;

		((std::get<Components*>(chunk.ptrToComponentArrayTuple)[index] = std::forward<Args>(componentValues)), ...);

		entityLocation.archetypeId = id;
		entityLocation.chunkIndex = static_cast<uint32_t>(archetypeChunks.size() - 1);
		entityLocation.indexInChunk = static_cast<uint32_t>(chunk.count);

		++chunk.count;
	}

	//Remove Entity
	void RemoveEntity(EntityLocation& entityLocation)
	{
		ArchetypeChunk& chunk = archetypeChunks[entityLocation.chunkIndex];
		const size_t lastIndexInChunk = chunk.count - 1;

		if (entityLocation.indexInChunk != lastIndexInChunk)
		{
			((std::get<Components*>(chunk.ptrToComponentArrayTuple)[entityLocation.indexInChunk] =
			 std::move(std::get<Components*>(chunk.ptrToComponentArrayTuple)[lastIndexInChunk])), ...);
		}

		--chunk.count;
	}
};