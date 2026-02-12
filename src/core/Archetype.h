#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
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
    std::vector<ArchetypeChunk> m_chunks;

    std::vector<TypeErasedBlockAllocator> m_allocators; // dens
    uint16_t m_densIds[MaxComponents];                  // dens Ids : Give Allocator Index -> Get ComponentId
    uint16_t m_sparse[MaxComponents];                   // sparse   : Give ComponentId     -> Get Allocator Index

    ArchetypeId m_archetypeId;
    ArchetypeComponentSignature m_componentSignature;
    std::string m_archetypeName;

    size_t m_chunkCapacity = 0;
    size_t m_totalSize = 0;

    ArchetypeChunk& CreateChunk()
    {
        m_chunks.emplace_back(m_chunkCapacity);
        ArchetypeChunk& newChunk = m_chunks.back();
        newChunk.components.resize(m_allocators.size());

        for (size_t i = 0; i < m_allocators.size(); ++i)
        {
            newChunk.components[i] = m_allocators[i].AllocateBlock();
        }

        return m_chunks.back();
    }

  public:
    ArchetypeId GetArchetypeId() const { return m_archetypeId; }
    ArchetypeComponentSignature GetComponentSignature() const { return m_componentSignature; }
    std::string GetArchetypeName() const { return m_archetypeName; }

    Archetype(ArchetypeId id, ArchetypeComponentSignature signature, std::string name, size_t chunkCapacity)
        : m_archetypeId(id), m_componentSignature(signature), m_archetypeName(name), m_chunkCapacity(chunkCapacity)
    {
        std::fill(std::begin(m_densIds), std::end(m_densIds), INT16_MAX);
        std::fill(std::begin(m_sparse), std::end(m_sparse), INT16_MAX);
    }

    template <typename... Components>
    void InitlizeComponentAllocators()
    {
        (
            [&]
            {
                ComponentID id = ComponentRegistry::GetComponentID<Components>();
                m_allocators.push_back(TypeErasedBlockAllocator(sizeof(Components), m_chunkCapacity));
                m_densIds[m_allocators.size() - 1] = id;
                m_sparse[id] = m_allocators.size() - 1;
            }(),
            ...);
    }

    template <typename Component>
    bool HasComponent()
    {
        ComponentID id = ComponentRegistry::GetComponentID<Component>();
        return m_sparse[id] < m_allocators.size() && m_sparse[id] == id;
    }

    template <typename Component>
    Component* TryGetComponent(uint32_t chunkIndex, uint32_t indexInChunk)
    {
        ComponentID id = ComponentRegistry::GetComponentID<Component>();
        if (id >= MaxComponents) return nullptr;

        uint16_t denseIndex = m_sparse[id];
        if (denseIndex >= m_allocators.size() || chunkIndex >= m_chunks.size()) return nullptr;

        ArchetypeChunk& chunk = m_chunks[chunkIndex];
        if (indexInChunk >= chunk.size) return nullptr;

        void* base = chunk.components[denseIndex];
        char* byteBase = static_cast<char*>(base);

        return reinterpret_cast<Component*>(byteBase + (indexInChunk * sizeof(Component)));
    }

    template <typename Component>
    Component& GetComponent(uint32_t chunkIndex, uint32_t indexInChunk)
    {
        Component* ptr = TryGetComponent<Component>(chunkIndex, indexInChunk);
        assert(ptr != nullptr && "Component not found");

        return *ptr;
    }

    template <typename... Components>
    EntityArchetypeLocation AddEntity(Entity entity, Components&&... components)
    {
        if (m_chunks.empty() || m_chunks.back().IsFull()) CreateChunk();

        ArchetypeChunk& targetChunk = m_chunks.back();
        const size_t index = targetChunk.size;

        (
            [&]
            {
                using Type = std::decay_t<Components>;
                ComponentID componentId = ComponentRegistry::GetComponentID<Type>();
                void* rawBlock = targetChunk.components[m_sparse[componentId]];
                Type* typeArray = reinterpret_cast<Type*>(rawBlock); // Cast raw block to an array of Type
                new (&typeArray[index]) Type(std::forward<Components>(components));
            }(),
            ...);

        ++targetChunk.size;
        ++m_totalSize;

        uint32_t chunkIndex = static_cast<uint32_t>(m_chunks.size() - 1);
        uint32_t indexInChunk = static_cast<uint32_t>(targetChunk.size - 1);
        targetChunk.entities[indexInChunk] = entity;

        return EntityArchetypeLocation{m_archetypeId, chunkIndex, indexInChunk};
    }

    std::pair<Entity, EntityArchetypeLocation> RemoveEntity(Entity entity, uint32_t chunkIndex, uint32_t indexInChunk)
    {
        ArchetypeChunk& chunk = m_chunks[chunkIndex];
        const size_t lastIndexInChunk = chunk.size - 1;

        if (indexInChunk != lastIndexInChunk)
        {
            for (size_t i = 0; i < m_allocators.size(); ++i)
            {
                void* rawBlock = chunk.components[i];
                char* base = static_cast<char*>(rawBlock);
                size_t componentSize = m_allocators[i].GetDataSize();

                std::memcpy(base + (indexInChunk * componentSize), base + (lastIndexInChunk * componentSize), componentSize);
            }
            chunk.entities[indexInChunk] = chunk.entities[lastIndexInChunk];
        }

        --chunk.size;
        --m_totalSize;

        // take a look later for case indexInChunk = lastIndexInChunk
        return {chunk.entities[indexInChunk], {m_archetypeId, chunkIndex, indexInChunk}};
    }
};
