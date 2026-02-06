#pragma once
#include <stdint.h>
#include <tuple>
#include <vector>
#include "ECSCommon.h"
#include "SlabAllocator.hpp"

// Address having new entities being added that the back chunk always.

struct BaseArchetype
{
    ArchetypeID id = InvalidArchetypeID;

    virtual EntityLocation AddEntity(void* componentTuple) = 0;

    virtual void RemoveEntity(uint32_t chunkIndex, uint32_t indexInChunk) = 0;

    virtual ~BaseArchetype() = default;
};

template <size_t ChunkSize, typename... Components>
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
        bool IsFull() const
        {
            return count == ChunkSize;
        }
    };

    std::vector<ArchetypeChunk> archetypeChunks;

    ArchetypeChunk& CreateArchetypeChunk()
    {
        ArchetypeChunk chunk;

        chunk.ptrToComponentArrayTuple =
            std::make_tuple(std::get<SlabAllocator<Components, ChunkSize>>(allocatorTuple).AllocateArray()...);

        archetypeChunks.push_back(chunk);
        return archetypeChunks.back();
    }

    template <typename Tuple, size_t... I>
    EntityLocation AddEntityImplementation(Tuple& componentValues, std::index_sequence<I...>)
    {
        if (archetypeChunks.empty() || archetypeChunks.back().IsFull()) CreateArchetypeChunk();

        ArchetypeChunk& chunk = archetypeChunks.back();
        const size_t index = chunk.count;

        ((std::get<Components*>(chunk.ptrToComponentArrayTuple)[index] = std::move(std::get<I>(componentValues))), ...);

        ++chunk.count;

        return EntityLocation{id, static_cast<uint32_t>(archetypeChunks.size() - 1), static_cast<uint32_t>(chunk.count - 1)};
    }

  public:
    EntityLocation AddEntity(void* componentTuple) override
    {
        return AddEntityImplementation(*static_cast<std::tuple<Components...>*>(componentTuple),
                                       std::index_sequence_for<Components...>{});
    }

    void RemoveEntity(uint32_t chunkIndex, uint32_t indexInChunk) override
    {
        ArchetypeChunk& chunk = archetypeChunks[chunkIndex];
        const size_t lastIndexInChunk = chunk.count - 1;

        if (indexInChunk != lastIndexInChunk)
        {
            ((std::get<Components*>(chunk.ptrToComponentArrayTuple)[indexInChunk] =
                  std::move(std::get<Components*>(chunk.ptrToComponentArrayTuple)[lastIndexInChunk])),
             ...);
        }

        --chunk.count;
    }
};
