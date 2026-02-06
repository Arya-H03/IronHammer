#include "ArchetypeRegistry.hpp"
#include "ECSCommon.h"
#include <cassert>
#include <vector>
#pragma onces

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

    std::vector<EntitySlot> entitySlots;
    std::vector<EntityLocation> entityLocations;

    ArchetypeRegistry archetypeRegistry;

    bool IsEntityValid(Entity entity) const
    {
        if (entity.id >= entitySlots.size()) return false;
        return entitySlots[entity.id].isOccupied && entitySlots[entity.id].generation == entity.generation;
    }

  public:
    EntityManager()
    {
        entitySlots.reserve(initialEntitySize);
        entityLocations.reserve(initialEntitySize);
    }

    template <typename... Components>
    Entity CreateEntity(Components&&... components)
    {
        // No free slots
        if (freeListHeadIndex == UINT32_MAX)
        {
            entitySlots.emplace_back(1, true, UINT32_MAX);
            entityLocations.emplace_back();

            uint32_t id = (unsigned int) entitySlots.size() - 1;
            return Entity{id, entitySlots[id].generation};
        }
        // Reuse a slot
        else
        {
            uint32_t newEntityIndex = freeListHeadIndex;
            EntitySlot& slot = entitySlots[newEntityIndex];
            freeListHeadIndex = slot.nextFreeEntityIndex;
            slot.isOccupied = true;
            ++slot.generation;

            entityLocations[newEntityIndex] = EntityLocation::InvalidLocation();

            return Entity{newEntityIndex, slot.generation};
        }

        // Find or Create Archetype

        // Build a tuple out of components

        // Pass tuple to the AddEntity()
    }

    void DeleteEntity(Entity entity)
    {
        assert(IsEntityValid(entity) && "Tried to delete invalid Entity");

        EntitySlot& slot = entitySlots[entity.id];
        slot.isOccupied = false;
        slot.nextFreeEntityIndex = freeListHeadIndex;

        entityLocations[entity.id] = EntityLocation::InvalidLocation();

        freeListHeadIndex = entity.id;
    }

    // Add Component to Entity

    // Remove Component from Entity
};
