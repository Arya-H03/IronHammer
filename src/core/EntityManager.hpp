#pragma once
#include "ArchetypeRegistry.hpp"
#include "ECSCommon.h"
#include <cassert>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>
class EntityManager
{
    struct EntitySlot
    {
        uint32_t generation = 1;
        bool isOccupied = false;
        uint32_t nextFreeEntityIndex = UINT32_MAX;
    };

  private:
    static constexpr size_t initialEntitySize = 1024;
    uint32_t freeListHeadIndex = UINT32_MAX;

    // Entity-id         e0  e1  e2  e3  ...
    // EntitySlot         0   1   2   3  ...
    // EntityLocation     0   1   2   3  ...
    std::vector<EntitySlot> entitySlots;
    std::vector<EntityArchetypeLocation> entityArchetypeLocations;

    void ValidateEntity(Entity entity) const
    {
        assert(entity.id < entitySlots.size() && "Trid to delete an Entity with an Id out of pool bounds");
        assert(entitySlots[entity.id].isOccupied && "Tried to delete a none occupied Entity");
        assert(entitySlots[entity.id].generation == entity.generation && "Tried to delete an Entity with miss match generation");
    }

  public:
    ArchetypeRegistry archetypeRegistry;
    EntityManager()
    {
        entitySlots.reserve(initialEntitySize);
        entityArchetypeLocations.reserve(initialEntitySize);
    }

    template <typename... Components>
    Entity CreateEntity(Components&&... components)
    {
        Entity newEntity;
        size_t newEntityArchetypeLocationIndex;
        // No free slots
        if (freeListHeadIndex == UINT32_MAX)
        {
            entitySlots.emplace_back(1, true, UINT32_MAX);
            entityArchetypeLocations.emplace_back();
            uint32_t id = (unsigned int) entitySlots.size() - 1;
            newEntity.id = id;
            newEntity.generation = entitySlots[id].generation;
            newEntityArchetypeLocationIndex = entityArchetypeLocations.size() - 1;
        }
        // Reuse a slot
        else
        {
            uint32_t id = freeListHeadIndex;
            EntitySlot& slot = entitySlots[id];
            freeListHeadIndex = slot.nextFreeEntityIndex;
            slot.isOccupied = true;
            ++slot.generation;
            newEntity.id = id;
            newEntity.generation = slot.generation;
            newEntityArchetypeLocationIndex = id;
        }

        BaseArchetype* archetype = archetypeRegistry.FindOrCreateArchetype<std::decay_t<Components>...>();
        auto componentTuple = std::make_tuple(std::forward<Components>(components)...);
        EntityArchetypeLocation newEntityArchetypeLocation = archetype->AddEntity(newEntity, &componentTuple);
        entityArchetypeLocations[newEntityArchetypeLocationIndex] = newEntityArchetypeLocation;

        return newEntity;
    }

    void DeleteEntity(Entity entity)
    {

        ValidateEntity(entity);

        // Recycle EntitySlot
        EntitySlot& slot = entitySlots[entity.id];
        slot.isOccupied = false;
        slot.nextFreeEntityIndex = freeListHeadIndex;
        freeListHeadIndex = entity.id;

        // Recycle EntityLocation
        EntityArchetypeLocation& currentArchetypeLocation = entityArchetypeLocations[entity.id];
        BaseArchetype* archetype = archetypeRegistry.GetArchetypeById(currentArchetypeLocation.archetypeId);
        std::pair<Entity, EntityArchetypeLocation> deletionResult = archetype->RemoveEntity(entity, currentArchetypeLocation.chunkIndex, currentArchetypeLocation.indexInChunk);
        entityArchetypeLocations[entity.id] = EntityArchetypeLocation::InvalidLocation();
        entityArchetypeLocations[deletionResult.first.id] = deletionResult.second;

    }

    // Add Component to Entity

    // Remove Component from Entity

};
