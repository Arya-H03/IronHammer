#pragma once
#include "core/CoreComponents.hpp"
#include "core/utils/Debug.h"
#include "ecs/World.h"
#include "ecs/archetype/Archetype.h"
#include "mold/MoldManager.h"

#include <utility>
#include <vector>

// Inline implementations for World class

inline World::World(MoldManager* moldManager) : m_moldManagerPtr(moldManager), m_entityManager(m_archetypeRegistry)
{
}

inline void World::UpdateWorld()
{
    m_commandBuffer.ExecuteAllCommands(m_entityManager);
}

inline ArchetypeRegistry& World::GetArchetypeRegistry()
{
    return m_archetypeRegistry;
}

inline EntityManager& World::GetEntityManager()
{
    return m_entityManager;
}

inline CommandBuffer& World::GetCommandBuffer()
{
    return m_commandBuffer;
}

inline bool World::ValidateEntity(Entity entity, bool log) const
{
    return m_entityManager.ValidateEntity(entity, log);
}

template <typename Func>
inline void World::ForEachComponentOfEntity(const EntityStorageLocation& entityLocation, Func&& func)
{
    Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
    archetype.ForEachComponent(entityLocation, func);
}

template <typename... QueryGroup>
inline Query* World::Query()
{
    return m_archetypeRegistry.GetOrCreateQuery<QueryGroup...>();
}

template <typename... Components>
inline Entity World::CreateEntityWithReturn(Components&&... components)
{
    return m_commandBuffer.CreateEntityFromComponentsWithReturn(m_entityManager, std::forward<Components>(components)...);
}

template <typename... Components>
inline void World::CreateEntityNoReturn(Components&&... components)
{
    m_commandBuffer.CreateEntityFromComponentsNoReturn(m_entityManager, std::forward<Components>(components)...);
}

inline std::vector<PendingComponent>& World::CreateEntityFromMoldObject(Mold& mold)
{
    std::vector<PendingComponent> pendingComponents = ComponentRegistry::GetAllPendingComponentsFromEntityJson(mold.entityJson);

    CMolded* entityTemplateComp = ComponentRegistry::GetComponentFromPendings<CMolded>(pendingComponents);
    if (!entityTemplateComp)
    {
        const ComponentInfo* componentInfoPtr = &ComponentRegistry::GetComponentInfo<CMolded>();
        CMolded* componentDataPtr = new CMolded(mold.entityName);
        pendingComponents.push_back(PendingComponent{componentInfoPtr, componentDataPtr});
    }
    else
    {
        LOG_WARNING("Tried to create entity from a template that has CEntityTemplate")
    }

    std::pair<Entity, std::vector<PendingComponent>&> result =
        m_commandBuffer.CreateEntityFromMold(m_entityManager, std::move(pendingComponents));
    mold.derivedEntities.push_back(result.first);

    return result.second;
}

inline std::vector<PendingComponent>* World::CreateEntityFromMoldName(const std::string& moldName)
{
    Mold* mold = m_moldManagerPtr->GetMoldByName(moldName);
    if (!mold)
    {
        LOG_WARNING("Tried to create entity from a mold that does not exist");
        return nullptr;
    }

    std::vector<PendingComponent> pendingComponents = ComponentRegistry::GetAllPendingComponentsFromEntityJson(mold->entityJson);

    CMolded* entityTemplateComp = ComponentRegistry::GetComponentFromPendings<CMolded>(pendingComponents);
    if (!entityTemplateComp)
    {
        const ComponentInfo* componentInfoPtr = &ComponentRegistry::GetComponentInfo<CMolded>();
        CMolded* componentDataPtr = new CMolded(mold->entityName);
        pendingComponents.push_back(PendingComponent{componentInfoPtr, componentDataPtr});
    }
    else
    {
        LOG_WARNING("Tried to create entity from a template that has CEntityTemplate")
    }

    std::pair<Entity, std::vector<PendingComponent>&> result =
        m_commandBuffer.CreateEntityFromMold(m_entityManager, std::move(pendingComponents));
    mold->derivedEntities.push_back(result.first);

    return &result.second;
}

inline void World::DestroyEntity(Entity entity)
{
    m_commandBuffer.DestroyEntity(entity);
}

template <typename Component>
inline void World::AddToEntity(Entity entity, Component&& component)
{
    m_commandBuffer.AddToEntity(entity, std::forward<Component>(component));
}

template <typename Component>
inline void World::RemoveFromEntity(Entity entity)
{
    m_commandBuffer.RemoveFromEntity<Component>(entity);
}

inline void World::AddToEntity(Entity entity, ComponentId componentId, void* componentPtr)
{
    m_commandBuffer.AddToEntity(entity, componentId, componentPtr);
}

inline void World::RemoveFromEntity(Entity entity, ComponentId componentId, void* componentPtr)
{
    m_commandBuffer.RemoveFromEntity(entity, componentId, componentPtr);
}

template <typename Component>
inline bool World::HasComponent(Entity entity)
{
    return m_entityManager.HasComponent<Component>(entity);
}

template <typename Component>
inline Component* World::TryGetComponent(Entity entity)
{
    return m_entityManager.TryGetComponent<Component>(entity);
}

inline void* World::TryGetComponent(Entity entity, ComponentId id)
{
    return m_entityManager.TryGetComponent(entity, id);
}

inline Json World::SerializeEntity(EntityStorageLocation entityLocation)
{
    return m_entityManager.SerializeEntity(entityLocation);
}

inline Json World::SerializeWorld()
{
    return m_entityManager.SerializeAllEntites();
}

inline void World::DeserializeWorld(Json worldJson)
{
    for (auto& entityJson : worldJson["entities"])
    {
        std::vector<PendingComponent> pendingComponents = ComponentRegistry::GetAllPendingComponentsFromEntityJson(entityJson);

        m_commandBuffer.CreateEntityFromMold(m_entityManager, std::move(pendingComponents));
    }
}
