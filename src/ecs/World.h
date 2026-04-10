#pragma once

#include "core/saving/JsonUtility.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/CommandBuffer.h"
#include "ecs/entity/EntityManager.hpp"
#include "mold/Mold.h"

#include <vector>

class MoldManager;

class World
{
    friend class Engine;

  private:
    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;
    CommandBuffer m_commandBuffer;
    MoldManager* m_moldManagerPtr;

    void UpdateWorld();

  public:
    World(MoldManager* moldManager);

    ArchetypeRegistry& GetArchetypeRegistry();
    EntityManager& GetEntityManager();
    CommandBuffer& GetCommandBuffer();

    bool ValidateEntity(Entity entity, bool log = true) const;

    template <typename Func>
    void ForEachComponentOfEntity(const EntityStorageLocation& entityLocation, Func&& func);

    template <typename... QueryGroup>
    Query* Query();

    template <typename... Components>
    Entity CreateEntityWithReturn(Components&&... components);

    template <typename... Components>
    void CreateEntityNoReturn(Components&&... components);

    std::vector<PendingComponent>& CreateEntityFromMoldObject(Mold& mold);
    std::vector<PendingComponent>* CreateEntityFromMoldName(const std::string& moldName);

    void DestroyEntity(Entity entity);

    template <typename Component>
    void AddToEntity(Entity entity, Component&& component);
    void AddToEntity(Entity entity, ComponentId componentId, void* componentPtr);

    template <typename Component>
    void RemoveFromEntity(Entity entity);
    void RemoveFromEntity(Entity entity, ComponentId componentId);

    template <typename Component>
    bool HasComponent(Entity entity);

    template <typename Component>
    Component* TryGetComponent(Entity entity);

    void* TryGetComponent(Entity entity, ComponentId id);

    Json SerializeEntity(EntityStorageLocation entityLocation);
    Json SerializeWorld();
    void DeserializeWorld(Json worldJson);
};

#include "World.inl"
