#pragma once

#include "core/CoreComponents.hpp"
#include "core/saving/JsonUtility.h"
#include "core/utils/Debug.h"
#include "ecs/archetype/Archetype.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/CommandBuffer.h"
#include "ecs/entity/EntityManager.hpp"
#include "mold/Mold.h"

#include <utility>
#include <vector>

class World
{
    friend class Engine;

  private:
    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;
    CommandBuffer m_commandBuffer;

    void UpdateWorld()
    {
        m_commandBuffer.ExecuteAllCommands(m_entityManager);
    }

  public:
    World() : m_entityManager(m_archetypeRegistry)
    {
    }
    ArchetypeRegistry& GetArchetypeRegistry()
    {
        return m_archetypeRegistry;
    }
    EntityManager& GetEntityManager()
    {
        return m_entityManager;
    }
    CommandBuffer& GetCommandBuffer()
    {
        return m_commandBuffer;
    }

    bool ValidateEntity(Entity entity, bool log = true) const
    {
        return m_entityManager.ValidateEntity(entity, log);
    }

    template <typename Func>
    void ForEachComponentOfEntity(const EntityStorageLocation& entityLocation, Func&& func)
    {
        Archetype& archetype = m_archetypeRegistry.GetArchetypeById(entityLocation.archetypeId);
        archetype.ForEachComponent(entityLocation, func);
    }

    template <typename... QueryGroup>
    Query* Query()
    {
        return m_archetypeRegistry.GetOrCreateQuery<QueryGroup...>();
    }

    template <typename... Components>
    Entity CreateEntityWithReturn(Components&&... components)
    {
        return m_commandBuffer.CreateEntityFromComponentsWithReturn(m_entityManager, std::forward<Components>(components)...);
    }

    template <typename... Components>
    void CreateEntityNoReturn(Components&&... components)
    {
        m_commandBuffer.CreateEntityFromComponentsNoReturn(m_entityManager, std::forward<Components>(components)...);
    }

    std::vector<PendingComponent>& CreateEntityFromMold(Mold& mold)
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
            LOG_WARNING("Tired to create entity from a template that has CEntityTemplate")
        }

        std::pair<Entity, std::vector<PendingComponent>&> result =
            m_commandBuffer.CreateEntityFromMold(m_entityManager, std::move(pendingComponents));
        mold.derivedEntities.push_back(result.first);

        return result.second;
    }

    void DestroyEntity(Entity entity)
    {
        m_commandBuffer.DestroyEntity(entity);
    }

    template <typename Component>
    void AddToEntity(Entity entity, Component&& component)
    {
        m_commandBuffer.AddToEntity(entity, std::forward<Component>(component));
    }

    template <typename Component>
    void RemoveFromEntity(Entity entity)
    {
        m_commandBuffer.RemoveFromEntity<Component>(entity);
    }

    void AddToEntity(Entity entity, ComponentId componentId, void* componentPtr)
    {
        m_commandBuffer.AddToEntity(entity, componentId, componentPtr);
    }

    void RemoveFromEntity(Entity entity, ComponentId componentId, void* componentPtr)
    {
        m_commandBuffer.RemoveFromEntity(entity, componentId, componentPtr);
    }

    template <typename Component>
    bool HasComponent(Entity entity)
    {
        return m_entityManager.HasComponent<Component>(entity);
    }

    template <typename Component>
    Component* TryGetComponent(Entity entity)
    {
        return m_entityManager.TryGetComponent<Component>(entity);
    }

    void* TryGetComponent(Entity entity, ComponentId id)
    {
        return m_entityManager.TryGetComponent(entity, id);
    }

    Json SerializeEntity(EntityStorageLocation entityLocation)
    {
        return m_entityManager.SerializeEntity(entityLocation);
    }
    Json SerializeWorld()
    {
        return m_entityManager.SerializeAllEntites();
    }

    void DeserializeWorld(Json worldJson)
    {
        for (auto& entityJson : worldJson["entities"])
        {
            std::vector<PendingComponent> pendingComponents = ComponentRegistry::GetAllPendingComponentsFromEntityJson(entityJson);

            m_commandBuffer.CreateEntityFromMold(m_entityManager, std::move(pendingComponents));
        }
    }
};
