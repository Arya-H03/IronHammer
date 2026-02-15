#pragma once
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include <cassert>

struct ArchetypeChunk
{
    std::vector<void*> components;   // Raw ptrs to memory blocks
    uint16_t densIds[MaxComponents]; // dens Ids : Give Components Index -> Get ComponentId
    uint16_t sparse[MaxComponents];  // sparse   : Give ComponentId     -> Get Components Index
    std::vector<Entity> entities;

    size_t capacity = 0;
    size_t size = 0;

    bool IsFull() const { return size == capacity; }

    ArchetypeChunk(size_t chunkCapacity) : capacity(chunkCapacity)
    {
        entities.resize(capacity);
        std::fill(std::begin(sparse), std::end(sparse), INT16_MAX);
        std::fill(std::begin(densIds), std::end(densIds), INT16_MAX);
    }

    template <typename Component>
    const Component* GetComponentRow() const
    {
        ComponentID id = ComponentRegistry::GetComponentID<Component>();
        // Invalid Id
        if (id >= MaxComponents) return nullptr;

        uint16_t denseIndex = sparse[id];

        assert(denseIndex < components.size() && "Invalid index in component[index]");

        uint16_t indexInComponents = sparse[id];
        void* rawBlock = components[indexInComponents];
        Component* typeArray = reinterpret_cast<Component*>(rawBlock);

        return typeArray;
    }
};
