#pragma once
#include <cassert>
#include <utility>
#include <vector>
#include <format>
#include <iostream>
#include "ecs/archetype/Archetype.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"

class EntityManager
{
    friend class CommandBuffer;
    friend class EntityInspectorWindow;

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
    std::vector<EntityStorageLocation> m_entityStorageLocations;
    ArchetypeRegistry& m_archetypeRegistry;

    bool ValidateEntity(Entity entity) const
    {
        bool isIdValid = entity.id < m_entitySlots.size();
        bool isEntityOccupied = m_entitySlots[entity.id].isOccupied;
        bool isGenValid = m_entitySlots[entity.id].generation == entity.generation;

#ifndef NDEBUG
        if (!isIdValid)
        {
            std::cerr << std::format("Tried to valid Entity({},{}) with invalid Id.", entity.id, entity.generation) << "\n";
        }
        if (!isEntityOccupied)
        {
            std::cerr << std::format("Tried to valid an occupied Entity({},{})", entity.id, entity.generation) << "\n";
        }
        if (!isGenValid)
        {
            std::cerr << std::format("Tried to valid Entity({},{}) with invalid Generation.", entity.id, entity.generation) << "\n";
        }
#endif

        return isIdValid && isEntityOccupied && isGenValid;
    }

    template <typename... Components>
    Entity CreateEntity(Components&&... components)
    {
        Entity newEntity;
        size_t newEntityArchetypeStorageIndex;

        // No free Entity Slots
        if (m_freeListHeadIndex == UINT32_MAX)
        {
            m_entitySlots.emplace_back(1, true, UINT32_MAX);
            m_entityStorageLocations.emplace_back();
            uint32_t id = (unsigned int) m_entitySlots.size() - 1;
            newEntity.id = id;
            newEntity.generation = m_entitySlots[id].generation;
            newEntityArchetypeStorageIndex = m_entityStorageLocations.size() - 1;
        }

        // Reuse an Entity Slot
        else
        {
            uint32_t id = m_freeListHeadIndex;
            EntitySlot& slot = m_entitySlots[id];
            m_freeListHeadIndex = slot.nextFreeEntityIndex;
            slot.isOccupied = true;
            ++slot.generation;
            newEntity.id = id;
            newEntity.generation = slot.generation;
            newEntityArchetypeStorageIndex = id;
        }

        // Find Archetype
        ComponentSignatureMask signature = m_archetypeRegistry.MakeSignatureMask<std::decay_t<Components>...>();
        Archetype& archetype = m_archetypeRegistry.GetArchetype(signature);
        EntityStorageLocation newEntityArchetypeStorage = archetype.AddEntity(newEntity, components...);

        // Update Entity Storage Location
        m_entityStorageLocations[newEntityArchetypeStorageIndex] = newEntityArchetypeStorage;

        return newEntity;
    }

    void DestroyEntity(Entity entity)
    {
        if (!ValidateEntity(entity)) return;

        // Recycle EntitySlot
        EntitySlot& slot = m_entitySlots[entity.id];
        slot.isOccupied = false;
        slot.nextFreeEntityIndex = m_freeListHeadIndex;
        m_freeListHeadIndex = entity.id;

        // Find Archetype
        EntityStorageLocation& currentEntityLocation = m_entityStorageLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(currentEntityLocation.archetypeId);

        std::pair<Entity, EntityStorageLocation> deletionResult = archetype.RemoveEntity(entity, currentEntityLocation);

        // Have to update the EntityStorageLocation of BOTH Entities.
        m_entityStorageLocations[deletionResult.first.id] = deletionResult.second;      // Swaped Entity
        m_entityStorageLocations[entity.id] = EntityStorageLocation::InvalidLocation(); // Poped Entity
    }

    template <typename Component>
    void AddToEntity(Entity entity, Component&& component)
    {
        if (!ValidateEntity(entity)) return;
        if (HasComponent<Component>(entity)) return;

        ComponentID componentId = ComponentRegistry::GetComponentID<Component>();

        // Find source and destination Archetypes
        Archetype& srcArchetype = m_archetypeRegistry.GetArchetypeById(m_entityStorageLocations[entity.id].archetypeId);
        ComponentSignatureMask distArchetypeSignature = srcArchetype.GetComponentSignature().set(componentId);
        Archetype& dstArchetype = m_archetypeRegistry.GetArchetype(distArchetypeSignature);

        // Migrate components from src to dst
        EntityStorageLocation& currentEntityLocation = m_entityStorageLocations[entity.id];
        EntityStorageLocation newEntityLocation = dstArchetype.MigrateComponentsFrom(srcArchetype, currentEntityLocation, entity);

        std::pair<Entity, EntityStorageLocation> deletionResult = srcArchetype.RemoveEntity(entity, currentEntityLocation);

        // Have to update the EntityStorageLocation of BOTH Entities.
        m_entityStorageLocations[deletionResult.first.id] = deletionResult.second; // Swaped Entity
        m_entityStorageLocations[entity.id] = newEntityLocation;                   // Poped Entity

        // Add new Component
        dstArchetype.ConstructComponentAt(std::forward<Component>(component), componentId, newEntityLocation);
    }

    // Note: currently you can have an Entity with no Components.
    // Consider having entities be destroyed in this senario.
    template <typename Component>
    void RemoveComponentFrom(Entity entity)
    {
        if (!ValidateEntity(entity)) return;
        if (!HasComponent<Component>(entity)) return;

        ComponentID componentId = ComponentRegistry::GetComponentID<Component>();

        // Find source and destination Archetypes
        Archetype& srcArchetype = m_archetypeRegistry.GetArchetypeById(m_entityStorageLocations[entity.id].archetypeId);
        ComponentSignatureMask distArchetypeSignature = srcArchetype.GetComponentSignature().reset(componentId);
        Archetype& dstArchetype = m_archetypeRegistry.GetArchetype(distArchetypeSignature);

        // Migrate components from src to dst
        EntityStorageLocation& currentEntityLocation = m_entityStorageLocations[entity.id];
        EntityStorageLocation newEntityLocation = dstArchetype.MigrateComponentsFrom(srcArchetype, currentEntityLocation, entity);

        std::pair<Entity, EntityStorageLocation> deletionResult = srcArchetype.RemoveEntity(entity, currentEntityLocation);

        // Have to update the EntityStorageLocation of BOTH Entities.
        m_entityStorageLocations[deletionResult.first.id] = deletionResult.second; // Swaped Entity
        m_entityStorageLocations[entity.id] = newEntityLocation;                   // Poped Entity
    }

  public:

    EntityManager(ArchetypeRegistry& archetypeRegistry) : m_archetypeRegistry(archetypeRegistry)
    {
        m_entitySlots.reserve(initialEntitySize);
        m_entityStorageLocations.reserve(initialEntitySize);
    }

    template <typename Component>
    bool HasComponent(Entity entity)
    {
        if (!ValidateEntity(entity)) return false;
        EntityStorageLocation& entityLocation = m_entityStorageLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.HasComponent<Component>();
    }

    template <typename Component>
    Component* TryGetComponent(Entity entity)
    {
        if (!ValidateEntity(entity)) return nullptr;
        EntityStorageLocation& entityLocation = m_entityStorageLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.GetComponentPtrByTemplate<Component>(entityLocation);
    }
};
