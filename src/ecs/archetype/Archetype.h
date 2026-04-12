#pragma once
#include "core/memory/TypeErasedBlockAllocator.hpp"
#include "ecs/archetype/ArchetypeChunk.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

// Forward declare
class WorldDebugger;

class Archetype
{
    friend class WorldDebugger;

  private:
    std::vector<ArchetypeChunk> m_chunks;

    std::vector<TypeErasedBlockAllocator> m_allocators; // dens
    uint16_t m_densIds[MaxComponents];                  // dens Ids : Give Allocator Index -> Get ComponentId
    uint16_t m_sparse[MaxComponents];                   // sparse   : Give ComponentId     -> Get Allocator Index

    ArchetypeId m_archetypeId;
    ComponentSignatureMask m_componentSignature;
    std::string m_archetypeName;

    size_t m_chunkCapacity = 0;
    size_t m_totalSize = 0;
    size_t m_currentFreeChunkIndex = SIZE_MAX;

    size_t CreateChunk()
    {
        m_chunks.emplace_back(m_chunkCapacity);
        ArchetypeChunk& newChunk = m_chunks.back();
        newChunk.components.resize(m_allocators.size());

        // Sets the component layout for chunk -> chunk.component[i] has the same
        // component as m_allocator[i]
        for (size_t i = 0; i < m_allocators.size(); ++i)
        {
            newChunk.components[i] = m_allocators[i].AllocateBlock();
            ComponentId id = m_densIds[i];
            newChunk.densIds[i] = id;
            newChunk.sparse[id] = i;
        }
        return m_chunks.size() - 1;
    }

    uint32_t GetFreeChunkIndex()
    {
        for (size_t i = 0; i < m_chunks.size(); ++i)
        {
            if(!m_chunks[i].IsFull()) return i;
        }

        return CreateChunk();
    }

  public:
    const ArchetypeId GetArchetypeId() const
    {
        return m_archetypeId;
    }
    const std::string& GetArchetypeName() const
    {
        return m_archetypeName;
    }

    const std::vector<ArchetypeChunk>& GetChunks() const
    {
        return m_chunks;
    }
    ComponentSignatureMask GetComponentSignature() const
    {
        return m_componentSignature;
    }

    bool HasComponent(ComponentId id) const
    {
        return m_sparse[id] < m_allocators.size() && m_densIds[m_sparse[id]] == id;
    }

    Archetype(ArchetypeId id, ComponentSignatureMask signature, std::string name, size_t chunkCapacity)
        : m_archetypeId(id), m_componentSignature(signature), m_archetypeName(name), m_chunkCapacity(chunkCapacity)
    {
        std::fill(std::begin(m_densIds), std::end(m_densIds), INT16_MAX);
        std::fill(std::begin(m_sparse), std::end(m_sparse), INT16_MAX);
    }

    bool ValidateEntityLocation(const EntityStorageLocation& entityLocation)
    {
        bool isChunkIndexValid = entityLocation.chunkIndex < m_chunks.size();
        bool isIndexInChunkValid = entityLocation.indexInChunk < m_chunks[entityLocation.chunkIndex].size;
#ifndef NDEBUG
        if (!isChunkIndexValid)
            LOG_WARNING(std::format("Invalid chunk index: {} | Should be at most {}", entityLocation.chunkIndex, m_chunks.size()));
        if (!isIndexInChunkValid)
            LOG_WARNING(std::format("Invalid index in chunk: {} | Should be at most {}", entityLocation.indexInChunk,
                                    m_chunks[entityLocation.indexInChunk].size));
#endif
        return isChunkIndexValid && isIndexInChunkValid;
    }

    void InitializeComponentAllocators(const ComponentSignatureMask& signature)
    {
        for (ComponentId id = 0; id < MaxComponents; ++id)
        {
            if (!signature.test(id)) continue; // If bit at id is 0

            const ComponentInfo& componentInfo = ComponentRegistry::GetComponentInfoById(id);

            m_allocators.push_back(TypeErasedBlockAllocator(componentInfo.size, m_chunkCapacity));
            m_densIds[m_allocators.size() - 1] = id;
            m_sparse[id] = m_allocators.size() - 1;
        }
    }

    template <typename Component>
    bool HasComponent()
    {
        ComponentId id = ComponentRegistry::GetComponentID<Component>();
        return m_sparse[id] < m_allocators.size() && m_densIds[m_sparse[id]] == id;
    }

    bool HasComponent(ComponentId id)
    {
        return m_sparse[id] < m_allocators.size() && m_densIds[m_sparse[id]] == id;
    }

    template <typename Component>
    Component* GetComponentPtrByTemplate(const EntityStorageLocation& entityLocation)
    {
        ComponentId id = ComponentRegistry::GetComponentID<Component>();
        if (id >= MaxComponents)
        {
            assert(false);
            return nullptr;
        }

        uint16_t denseIndex = m_sparse[id];
        assert(denseIndex < m_allocators.size() && "Tried to access a component not in the archetype");

        if (entityLocation.chunkIndex >= m_chunks.size())
        {
            assert(false);
            return nullptr;
        }

        ArchetypeChunk& chunk = m_chunks[entityLocation.chunkIndex];
        if (entityLocation.indexInChunk >= chunk.size)
        {
            assert(false);
            return nullptr;
        }

        void* base = chunk.components[denseIndex];
        char* byteBase = static_cast<char*>(base);

        return reinterpret_cast<Component*>(byteBase + (entityLocation.indexInChunk * sizeof(Component)));
    }

    void* GetComponentPtrById(const EntityStorageLocation& entityLocation, ComponentId componentId)
    {
        if (componentId >= MaxComponents) return nullptr;
        if (m_sparse[componentId] >= m_allocators.size()) return nullptr;

        ComponentInfo componentInfo = ComponentRegistry::GetComponentInfoById(componentId);
        void* rawBlock = m_chunks[entityLocation.chunkIndex].components[m_sparse[componentId]];
        char* byteBase = static_cast<char*>(rawBlock);
        void* componentPtr = byteBase + (entityLocation.indexInChunk * componentInfo.size);

        return componentPtr;
    }

    template <typename Func>
    void ForEachComponent(const EntityStorageLocation& entityLocation, Func&& func)
    {
        if (!ValidateEntityLocation(entityLocation)) return;

        for (size_t i = 0; i < m_allocators.size(); ++i)
        {
            ComponentId componentId = m_densIds[i];
            ComponentInfo componentInfo = ComponentRegistry::GetComponentInfoById(componentId);
            void* rawBlock = m_chunks[entityLocation.chunkIndex].components[m_sparse[componentId]];
            char* byteBase = static_cast<char*>(rawBlock);
            void* componentPtr = byteBase + (entityLocation.indexInChunk * componentInfo.size);
            func(componentId, componentPtr);
        }
    }

    template <typename Component>
    void ConstructComponentByType(Component&& component, ComponentId id, const EntityStorageLocation& entityLocation)
    {
        size_t allocatorIndex = m_sparse[id];
        void* rawBlock = m_chunks[entityLocation.chunkIndex].components[allocatorIndex];
        Component* componentArray = reinterpret_cast<Component*>(rawBlock);

        new (&componentArray[entityLocation.indexInChunk]) Component(std::forward<Component>(component));
    }

    void ConstructComponentById(void* componentPtr, const ComponentInfo& componentInfo, const EntityStorageLocation& entityLocation)
    {
        size_t allocatorIndex = m_sparse[componentInfo.id];
        void* rawBlock = m_chunks[entityLocation.chunkIndex].components[allocatorIndex];
        componentInfo.EmplaceComponent(rawBlock, componentPtr, entityLocation.indexInChunk);
    }

    // Do later: If archetypes are stable, you can precompute a component map allowing
    // components to move without need of index or chunk look up between archetypes.
    EntityStorageLocation MigrateComponentsFrom(Archetype& srcArchetype, EntityStorageLocation& entityLocation, Entity entity)
    {
        assert(&srcArchetype != this);

        ArchetypeChunk& srcChunk = srcArchetype.m_chunks[entityLocation.chunkIndex];
        uint32_t chunkIndex = GetFreeChunkIndex();
        ArchetypeChunk& dstChunk = m_chunks[chunkIndex];

        uint32_t srcIndex = entityLocation.indexInChunk;
        uint32_t dstIndex = dstChunk.size;

        for (size_t i = 0; i < srcChunk.components.size(); ++i)
        {
            ComponentId id = srcArchetype.m_densIds[i];
            if (!HasComponent(id)) continue;

            const ComponentInfo& componentInfo = ComponentRegistry::GetComponentInfoById(id);

            size_t dstAllocatorIndex = m_sparse[id];

            void* srcRawBlock = srcChunk.components[i];
            void* dstRawBlock = dstChunk.components[dstAllocatorIndex];

            componentInfo.MoveComponent(srcRawBlock, dstRawBlock, srcIndex, dstIndex);
        }

        ++dstChunk.size;
        ++m_totalSize;

        dstChunk.entities[dstIndex] = entity;

        return EntityStorageLocation{m_archetypeId, chunkIndex, dstIndex};
    }

    template <typename... Components>
    EntityStorageLocation AddEntity(Entity entity, Components&&... components)
    {
        const uint32_t freeChunkIndex = GetFreeChunkIndex();
        ArchetypeChunk& freeChunk = m_chunks[freeChunkIndex];
        (
            [&]
            {
                using Type = std::decay_t<Components>;

                ComponentId componentId = ComponentRegistry::GetComponentID<Type>();
                void* rawBlock = freeChunk.components[m_sparse[componentId]];
                Type* typeArray = reinterpret_cast<Type*>(rawBlock); // Cast raw block to an array of Type

                new (&typeArray[freeChunk.size]) Type(std::forward<Components>(components));
            }(),
            ...);

        ++freeChunk.size;
        ++m_totalSize;

        uint32_t indexInChunk = static_cast<uint32_t>(freeChunk.size - 1);

        freeChunk.entities[indexInChunk] = entity;

        return EntityStorageLocation{m_archetypeId, freeChunkIndex, indexInChunk};
    }

    EntityStorageLocation AddEntity(Entity entity, std::vector<PendingComponent>& pendingComponents)
    {
        const uint32_t freeChunkIndex = GetFreeChunkIndex();
        ArchetypeChunk& freeChunk = m_chunks[freeChunkIndex];

        for (const auto& component : pendingComponents)
        {
            ComponentId componentId = component.componentInfoPtr->id;
            void* rawBlock = freeChunk.components[m_sparse[componentId]];
            component.componentInfoPtr->EmplaceComponent(rawBlock, component.componentDataPtr, freeChunk.size);
        }

        ++freeChunk.size;
        ++m_totalSize;

        uint32_t indexInChunk = static_cast<uint32_t>(freeChunk.size - 1);

        freeChunk.entities[indexInChunk] = entity;

        return EntityStorageLocation{m_archetypeId, freeChunkIndex, indexInChunk};
    }

    // A Swap Pop happens in RemoveEntity(...). The returned pair is {SwapedEntity,
    // newSwapedEntityStorageLocation}.
    std::pair<Entity, EntityStorageLocation> RemoveEntity(Entity entity, EntityStorageLocation& entityLocation)
    {
        uint32_t chunkIndex = entityLocation.chunkIndex;
        uint32_t indexInChunk = entityLocation.indexInChunk;

        ArchetypeChunk& chunk = m_chunks[chunkIndex];

        const size_t lastIndexInChunk = chunk.size - 1;

        if (indexInChunk != lastIndexInChunk)
        {
            for (size_t i = 0; i < m_allocators.size(); ++i)
            {
                ComponentId id = m_densIds[i];
                void* rawBlock = chunk.components[i];

                const ComponentInfo& componentInfo = ComponentRegistry::GetComponentInfoById(id);
                componentInfo.MoveComponent(rawBlock, rawBlock, lastIndexInChunk, indexInChunk);
            }
            chunk.entities[indexInChunk] = chunk.entities[lastIndexInChunk];
        }

        --chunk.size;
        --m_totalSize;

        return {chunk.entities[indexInChunk], {m_archetypeId, chunkIndex, indexInChunk}};
    }
};
