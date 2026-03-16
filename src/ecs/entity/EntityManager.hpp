#pragma once
#include <cassert>
#include <utility>
#include <vector>
#include <format>
#include "core/utils/Debug.h"
#include "ecs/archetype/Archetype.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "core/saving/JsonUtility.h"

class EntityManager
{
    friend class OldCommandBuffer;

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

  public:

    EntityManager(ArchetypeRegistry& archetypeRegistry) : m_archetypeRegistry(archetypeRegistry)
    {
        m_entitySlots.reserve(initialEntitySize);
        m_entityStorageLocations.reserve(initialEntitySize);
    }

    const std::vector<EntityStorageLocation>& GetAllEntityLocations() const { return m_entityStorageLocations; }

    EntityStorageLocation GetEntityLocation(Entity entity) const
    {
        if (!ValidateEntity(entity)) return EntityStorageLocation {};
        return m_entityStorageLocations[entity.id];
    }

    Archetype* GetEntityArchetypePtr(Entity entity) const
    {
        if (!ValidateEntity(entity)) return nullptr;
        return &m_archetypeRegistry.GetArchetypeById(GetEntityLocation(entity).archetypeId);
    }

    bool ValidateEntity(Entity entity, bool log = true) const
    {
        bool isIdValid = entity.id < m_entitySlots.size();
        if (!isIdValid)
        {
#ifndef NDEBUG
            if (log) LOG_WARNING(std::format("Tried to valid Entity({},{}) with invalid Id.", entity.id, entity.generation));
#endif
            return false;
        }
        bool isEntityOccupied = m_entitySlots[entity.id].isOccupied;
        if (!isEntityOccupied)
        {
#ifndef NDEBUG
            if (log) LOG_WARNING(std::format("Tried to valid an occupied Entity({},{})", entity.id, entity.generation));
#endif
            return false;
        }
        bool isGenValid = m_entitySlots[entity.id].generation == entity.generation;
        if (!isGenValid)
        {
#ifndef NDEBUG
            if (log) LOG_WARNING(std::format("Tried to valid Entity({},{}) with invalid Generation.", entity.id, entity.generation));
#endif
            return false;
        }
        return true;
    }

    template <typename Component>
    bool HasComponent(Entity entity)
    {
        if (!ValidateEntity(entity)) return false;
        EntityStorageLocation& entityLocation = m_entityStorageLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.HasComponent<Component>();
    }

    bool HasComponent(Entity entity, ComponentId componentId)
    {
        if (!ValidateEntity(entity)) return false;
        EntityStorageLocation& entityLocation = m_entityStorageLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.HasComponent(componentId);
    }

    template <typename Component>
    Component* TryGetComponent(Entity entity)
    {
        if (!ValidateEntity(entity)) return nullptr;
        EntityStorageLocation& entityLocation = m_entityStorageLocations[entity.id];
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        return archetype.GetComponentPtrByTemplate<Component>(entityLocation);
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
        ComponentSignatureMask signature = ComponentRegistry::MakeSignatureMask<std::decay_t<Components>...>();
        Archetype& archetype = m_archetypeRegistry.GetArchetype(signature);
        EntityStorageLocation newEntityArchetypeStorage = archetype.AddEntity(newEntity, components...);

        // Update Entity Storage Location
        m_entityStorageLocations[newEntityArchetypeStorageIndex] = newEntityArchetypeStorage;
        return newEntity;
    }

    Entity CreateEntity(std::vector<PendingComponent>& pendingComponents)
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
        ComponentSignatureMask signature = ComponentRegistry::MakeSignatureMask(pendingComponents);
        Archetype& archetype = m_archetypeRegistry.GetArchetype(signature);
        EntityStorageLocation newEntityArchetypeStorage = archetype.AddEntity(newEntity, pendingComponents);

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

        ComponentId componentId = ComponentRegistry::GetComponentID<Component>();

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
        m_entityStorageLocations[entity.id] = newEntityLocation;                   // mutable Poped Entity

        // Add new Component
        dstArchetype.ConstructComponentByType(std::forward<Component>(component), componentId, newEntityLocation);
    }

    void AddToEntity(Entity entity, ComponentId componentId, void* componentPtr)
    {
        if (!ValidateEntity(entity)) return;
        if (HasComponent(entity, componentId)) return;

        const ComponentInfo& componentInfo = ComponentRegistry::GetComponentInfoById(componentId);

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
        dstArchetype.ConstructComponentById(componentPtr, componentInfo, newEntityLocation);

        // Destroy pointer
        componentInfo.DestroyComponent(componentPtr);
    }

    // Note: currently you can have an Entity with no Components.
    // Consider having entities be destroyed in this senario.
    template <typename Component>
    void RemoveComponentFrom(Entity entity)
    {
        if (!ValidateEntity(entity)) return;
        if (!HasComponent<Component>(entity)) return;

        ComponentId componentId = ComponentRegistry::GetComponentID<Component>();

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

    void RemoveComponentFrom(Entity entity, ComponentId componentId, void* componentPtr)
    {
        if (!ValidateEntity(entity)) return;
        if (!HasComponent(entity, componentId)) return;

        const ComponentInfo& componentInfo = ComponentRegistry::GetComponentInfoById(componentId);

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

    Json SerializeEntity(EntityStorageLocation entityLocation)
    {
        Json entityJson;

        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        archetype.ForEachComponent(entityLocation,
            [&](ComponentId id, void* ptr)
            {
                const ComponentInfo& componentInfo = ComponentRegistry::GetComponentInfoById(id);
                componentInfo.SerializeComponent(entityJson, ptr);
            });

        return entityJson;
    }

    Json SerializeAllEntites()
    {
        Json allEntitiesJson;

        for (auto& entityLocation : m_entityStorageLocations)
        {
            if (entityLocation.archetypeId == InvalidArchetypeID) continue;
            Json entityJson = SerializeEntity(entityLocation);
            allEntitiesJson["entities"].push_back(entityJson);
        }

        return allEntitiesJson;
    }
};
