#pragma once

#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/component/Components.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityCommands.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "ecs/entity/EntityInspector.hpp"

class World
{
    friend class Engine;

  private:

    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;
    CommandBuffer m_commandBuffer;

    EntityInspector m_entityInspector;
    const ArchetypeDebugger m_archetypeDebugger;

    void UpdateWorld() { m_commandBuffer.ExecuteAllCommands(); }

  public:

    World()
        : m_entityManager(m_archetypeRegistry)
        , m_commandBuffer(m_entityManager)
        , m_entityInspector(m_entityManager)
        , m_archetypeDebugger(m_archetypeRegistry, m_commandBuffer, m_entityInspector)
    {
    }

    EntityInspector& GetEntityInspector() { return m_entityInspector; }
    const ArchetypeDebugger& GetArchetypeDebugger() const { return m_archetypeDebugger; }

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
};
