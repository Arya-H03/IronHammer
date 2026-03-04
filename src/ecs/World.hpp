#pragma once

#include "ecs/archetype/Archetype.h"

#include "ecs/common/ECSCommon.h"
#include "ecs/component/Components.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityCommands.hpp"
#include "ecs/entity/EntityManager.hpp"


class World
{
    friend class Engine;

  private:

    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;
    CommandBuffer m_commandBuffer;

    void UpdateWorld() { m_commandBuffer.ExecuteAllCommands(); }

  public:

    World() : m_entityManager(m_archetypeRegistry), m_commandBuffer(m_entityManager) { }
    ArchetypeRegistry& GetArchetypeRegistry() { return m_archetypeRegistry; }
    EntityManager& GetEntityManager() { return m_entityManager; }
    CommandBuffer& GetCommandBuffer() { return m_commandBuffer; }

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
    void CreateEntity(Entity& entity, Components&&... components)
    {
        m_commandBuffer.CreateEntity(entity, std::forward<Components>(components)...);
    }

    template <typename... Components>
    void CreateEntity(Components&&... components)
    {
        m_commandBuffer.CreateEntity(std::forward<Components>(components)...);
    }

    void DestroyEntity(Entity entity) { m_commandBuffer.DestroyEntity(entity); }

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

    Json SerializeWorld() { return m_entityManager.SerializeAllEntites(); }

    void DeserializeWorld(Json worldJson) { m_entityManager.DeserializeWorld(worldJson); }

    Json SerializeEntity(EntityStorageLocation entityLocation) { return m_entityManager.SerializeEntity(entityLocation); }

    void DeserializeEntity(Json entityJson) { m_entityManager.DeserializeEntity(entityJson); }
};
