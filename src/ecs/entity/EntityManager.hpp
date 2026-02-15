#pragma once
#include <cassert>
#include <utility>
#include <vector>
#include "ecs/archetype/Archetype.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/common/ECSCommon.h"
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
    uint32_t m_freeListHeadIndex = UINT32_MAX;

    // Entity-id         e0  e1  e2  e3  ...
    // EntitySlot         0   1   2   3  ...
    // EntityLocation     0   1   2   3  ...
    std::vector<EntitySlot> m_entitySlots;
    std::vector<EntityStorageLocation> m_entityArchetypeLocations;
    ArchetypeRegistry& m_archetypeRegistry;
    void ValidateEntity(Entity entity) const
    {
        assert(entity.id < m_entitySlots.size() && "Trid to delete an Entity with an Id out of pool bounds");
        assert(m_entitySlots[entity.id].isOccupied && "Tried to delete a none occupied Entity");
        assert(m_entitySlots[entity.id].generation == entity.generation
               && "Tried to delete an Entity with miss match generation");
    }

  public:
    EntityManager(ArchetypeRegistry& archetypeRegistry): m_archetypeRegistry(archetypeRegistry)
    {
        m_entitySlots.reserve(initialEntitySize);
        m_entityArchetypeLocations.reserve(initialEntitySize);
    }

    template <typename... Components>
    Entity CreateEntity(Components&&... components)
    {
        Entity newEntity;
        size_t newEntityArchetypeLocationIndex;
        // No free slots
        if (m_freeListHeadIndex == UINT32_MAX)
        {
            m_entitySlots.emplace_back(1, true, UINT32_MAX);
            m_entityArchetypeLocations.emplace_back();
            uint32_t id = (unsigned int) m_entitySlots.size() - 1;
            newEntity.id = id;
            newEntity.generation = m_entitySlots[id].generation;
            newEntityArchetypeLocationIndex = m_entityArchetypeLocations.size() - 1;
        }
        // Reuse a slot
        else
        {
            uint32_t id = m_freeListHeadIndex;
            EntitySlot& slot = m_entitySlots[id];
            m_freeListHeadIndex = slot.nextFreeEntityIndex;
            slot.isOccupied = true;
            ++slot.generation;
            newEntity.id = id;
            newEntity.generation = slot.generation;
            newEntityArchetypeLocationIndex = id;
        }

        ComponentSignatureMask signature = m_archetypeRegistry.MakeArchetypeSignature<std::decay_t<Components>...>();
        Archetype& archetype = m_archetypeRegistry.FindOrCreateArchetype(signature);

        EntityStorageLocation newEntityArchetypeLocation = archetype.AddEntity(newEntity, components...);
        m_entityArchetypeLocations[newEntityArchetypeLocationIndex] = newEntityArchetypeLocation;

        return newEntity;
    }

    void DeleteEntity(Entity entity)
    {
        ValidateEntity(entity);

        // Recycle EntitySlot
        EntitySlot& slot = m_entitySlots[entity.id];
        slot.isOccupied = false;
        slot.nextFreeEntityIndex = m_freeListHeadIndex;
        m_freeListHeadIndex = entity.id;

        // Recycle EntityLocation
        EntityStorageLocation& currentEntityLocation = m_entityArchetypeLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(currentEntityLocation.archetypeId);
        std::pair<Entity, EntityStorageLocation> deletionResult = archetype.RemoveEntity(entity, currentEntityLocation);
        m_entityArchetypeLocations[entity.id] = EntityStorageLocation::InvalidLocation();
        m_entityArchetypeLocations[deletionResult.first.id] = deletionResult.second;
    }

    template <typename Component>
    void AddToEntity(Entity entity, Component&& component)
    {
        ValidateEntity(entity);
        if (HasComponent<Component>(entity)) return;

        ComponentID componentId = ComponentRegistry::GetComponentID<Component>();

        Archetype& srcArchetype = m_archetypeRegistry.GetArchetypeById(m_entityArchetypeLocations[entity.id].archetypeId);
        ComponentSignatureMask distArchetypeSignature = srcArchetype.GetComponentSignature().set(componentId);
        Archetype& dstArchetype = m_archetypeRegistry.FindOrCreateArchetype(distArchetypeSignature);

        EntityStorageLocation& currentEntityLocation = m_entityArchetypeLocations[entity.id];
        EntityStorageLocation newEntityLocation =
            dstArchetype.MigrateComponentsFrom(srcArchetype, currentEntityLocation, entity);

        srcArchetype.RemoveEntity(entity, currentEntityLocation);
        dstArchetype.ConstructComponentAt(std::forward<Component>(component), componentId, newEntityLocation);

        m_entityArchetypeLocations[entity.id] = newEntityLocation;
    }

    // Note: currently you can have an Entity with no Components.
    // Consider having entities be destroyed in this senario.
    template <typename Component>
    void RemoveComponentFrom(Entity entity)
    {
        ValidateEntity(entity);
        if (!HasComponent<Component>(entity)) return;

        ComponentID componentId = ComponentRegistry::GetComponentID<Component>();

        Archetype& srcArchetype = m_archetypeRegistry.GetArchetypeById(m_entityArchetypeLocations[entity.id].archetypeId);
        ComponentSignatureMask distArchetypeSignature = srcArchetype.GetComponentSignature().reset(componentId);
        Archetype& dstArchetype = m_archetypeRegistry.FindOrCreateArchetype(distArchetypeSignature);

        EntityStorageLocation& currentEntityLocation = m_entityArchetypeLocations[entity.id];
        EntityStorageLocation newEntityLocation =
            dstArchetype.MigrateComponentsFrom(srcArchetype, currentEntityLocation, entity);

        srcArchetype.RemoveEntity(entity, currentEntityLocation);

        m_entityArchetypeLocations[entity.id] = newEntityLocation;
    }

    template <typename Component>
    bool HasComponent(Entity entity)
    {
        ValidateEntity(entity);
        EntityStorageLocation& entityLocation = m_entityArchetypeLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.HasComponent<Component>();
    }

    template <typename Component>
    Component* GetComponentPtr(Entity entity)
    {
        ValidateEntity(entity);
        EntityStorageLocation& entityLocation = m_entityArchetypeLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.TryGetComponent<Component>(entityLocation.chunkIndex, entityLocation.indexInChunk);
    }

    template <typename Component>
    Component& GetComponentRef(Entity entity)
    {
        ValidateEntity(entity);
        EntityStorageLocation& entityLocation = m_entityArchetypeLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.GetComponent<Component>(entityLocation.chunkIndex, entityLocation.indexInChunk);
    }
};
