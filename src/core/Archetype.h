#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>
#include "ComponentRegistry.hpp"
#include "ECSCommon.h"
#include "TypeErasedBlockAllocator.hpp"

struct ArchetypeChunk
{
    std::vector<void*> components; // Raw ptrs to memory blocks
    std::vector<Entity> entities;

    size_t capacity = 0;
    size_t size = 0;

    bool IsFull() const { return size == capacity; }

    ArchetypeChunk(size_t chunkCapacity) : capacity(chunkCapacity) { entities.resize(capacity); }
};

// Forward declare
class ArchetypeDebugger;

class Archetype
{
    friend class ArchetypeDebugger;

  private:
    std::vector<ArchetypeChunk> chunks;

    std::vector<TypeErasedBlockAllocator> allocators; // dens
    uint16_t densIds[MaxComponents];                  // dens Ids
    uint16_t sparse[MaxComponents];                   // sparse

    ArchetypeID archetypeId;
    size_t m_chunkCapacity = 0;
    size_t m_totalSize = 0;

    ArchetypeChunk& CreateChunk()
    {
        chunks.emplace_back(m_chunkCapacity);
        ArchetypeChunk& newChunk = chunks.back();
        newChunk.components.resize(allocators.size());

        for (size_t i = 0; i < allocators.size(); ++i)
        {
            newChunk.components[i] = allocators[i].AllocateBlock();
        }

        return chunks.back();
    }

  public:
    ArchetypeID GetArchetypeId() const { return archetypeId; }
    void SetArchetypeId(ArchetypeID id) { archetypeId = id; }

    template <typename... Components>
    void InitAllocators()
    {
        (
            [&]
            {
                ComponentID id = ComponentRegistry::GetComponentID<Components>();
                allocators.push_back(TypeErasedBlockAllocator(sizeof(Components), m_chunkCapacity));
                densIds[allocators.size() - 1] = id;
                sparse[id] = allocators.size() - 1;
            }(),
            ...);
    }
    Archetype(size_t chunkCapacity) : m_chunkCapacity(chunkCapacity) {}

    template <typename... Components>
    EntityArchetypeLocation AddEntity(Entity entity, Components&&... components)
    {
        if (chunks.empty() || chunks.back().IsFull()) CreateChunk();

        ArchetypeChunk& targetChunk = chunks.back();
        const size_t index = targetChunk.size;

        (
            [&]
            {
                using Type = std::decay_t<Components>;
                ComponentID componentId = ComponentRegistry::GetComponentID<Type>();
                void* rawBlock = targetChunk.components[sparse[componentId]];
                Type* typeArray = reinterpret_cast<Type*>(rawBlock); // Cast raw block to an array of Type
                new (&typeArray[index]) Type(std::forward<Components>(components));
            }(),
            ...);

        ++targetChunk.size;
        ++m_totalSize;

        uint32_t chunkIndex = static_cast<uint32_t>(chunks.size() - 1);
        uint32_t indexInChunk = static_cast<uint32_t>(targetChunk.size - 1);
        targetChunk.entities[indexInChunk] = entity;

        return EntityArchetypeLocation{archetypeId, chunkIndex, indexInChunk};
    }

    std::pair<Entity, EntityArchetypeLocation> RemoveEntity(Entity entity, uint32_t chunkIndex, uint32_t indexInChunk)
    {
        ArchetypeChunk& chunk = chunks[chunkIndex];
        const size_t lastIndexInChunk = chunk.size - 1;

        if (indexInChunk != lastIndexInChunk)
        {
            for (size_t i = 0; i < allocators.size(); ++i)
            {
                void* rawBlock = chunk.components[i];
                char* base = static_cast<char*>(rawBlock);
                size_t componentSize = allocators[i].GetDataSize();

                std::memcpy(base + (indexInChunk * componentSize), base + (lastIndexInChunk * componentSize), componentSize);
            }
            chunk.entities[indexInChunk] = chunk.entities[lastIndexInChunk];
        }

        --chunk.size;
        --m_totalSize;

        // take a look later for case indexInChunk = lastIndexInChunk
        return {chunk.entities[indexInChunk], {archetypeId, chunkIndex, indexInChunk}};
    }
};
