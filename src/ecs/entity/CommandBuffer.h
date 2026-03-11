#pragma once

#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"
#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>
#include <vector>

// Optimize Later:
// Remove std::function entirely
// Store commands in contiguous memory
// Sort commands by archetype signature before execution
// Allow command buffers per system/thread

class CommandBuffer
{
  private:

    struct CreateEntityCommandTypeErased
    {
        Entity* returnEntity;
        std::vector<PendingComponent> pendingComponents;
    };

    struct CreateEntityCommandTemplated
    {
        Entity* returnEntity;
        std::function<Entity(EntityManager&)> CreateFn;
    };

    struct DestroyEntityCommand
    {
        Entity entity;
    };

    struct AddToEntityCommandTemplated
    {
        Entity entity;
        std::function<void(EntityManager&)> AddFn;
    };

    struct AddToEntityCommandTypeErased
    {
        Entity entity;
        ComponentID componentID;
        void* componentPtr;
    };

    struct RemoveFromEntityCommandTemplated
    {
        Entity entity;
        std::function<void(EntityManager&)> RemoveFn;
    };

    struct RemoveFromEntityCommandTypeErased
    {
        Entity entity;
        ComponentID componentID;
        void* componentPtr;
    };

    std::vector<CreateEntityCommandTypeErased> m_createEntityCommandsTypeErased;
    std::vector<CreateEntityCommandTemplated> m_createEntityCommandsTemplated;
    std::vector<DestroyEntityCommand> m_destroyEntityCommands;
    std::vector<AddToEntityCommandTemplated> m_addToEntityCommandsTemplated;
    std::vector<AddToEntityCommandTypeErased> m_addToEntityCommandsTypeErased;
    std::vector<RemoveFromEntityCommandTemplated> m_removeFromEntityCommandsTemplated;
    std::vector<RemoveFromEntityCommandTypeErased> m_removeFromEntityCommandsTypeErased;

    void Clear()
    {
        m_createEntityCommandsTypeErased.clear();
        m_createEntityCommandsTemplated.clear();
        m_destroyEntityCommands.clear();
        m_addToEntityCommandsTemplated.clear();
        m_addToEntityCommandsTypeErased.clear();
        m_removeFromEntityCommandsTemplated.clear();
        m_removeFromEntityCommandsTypeErased.clear();
    }

  public:

    CommandBuffer() = default;

    // Create from EntityTemplate ///////
    std::vector<PendingComponent>& CreateEntityFromTemplate(std::vector<PendingComponent>&& components)
    {
        m_createEntityCommandsTypeErased.push_back({ nullptr, std::move(components) });
        auto& command = m_createEntityCommandsTypeErased.back();
        return command.pendingComponents;
    }

    std::vector<PendingComponent>& CreateEntityFromTemplate(Entity entity, std::vector<PendingComponent>&& components)
    {
        m_createEntityCommandsTypeErased.push_back({ &entity, std::move(components) });
        auto& command = m_createEntityCommandsTypeErased.back();
        return command.pendingComponents;
    }
    //////////////////////////////////////

    // Create from Components ////////////
    template <typename... Components>
    void CreateEntityFromComponents(Components&&... components)
    {
        auto tuple = std::make_tuple(std::forward<Components>(components)...);

        m_createEntityCommandsTemplated.push_back({ nullptr, [tuple = std::move(tuple)](EntityManager& entityManager) mutable {
                                                       return std::apply([&](auto&&... comps)
                                                           { return entityManager.CreateEntity(std::forward<decltype(comps)>(comps)...); },
                                                           std::move(tuple));
                                                   } });
    }

    template <typename... Components>
    void CreateEntityFromComponents(Entity entity, Components&&... components)
    {
        auto tuple = std::make_tuple(std::forward<Components>(components)...);

        m_createEntityCommandsTemplated.push_back({ &entity, [tuple = std::move(tuple)](EntityManager& entityManager) mutable {
                                                       return std::apply([&](auto&&... comps)
                                                           { return entityManager.CreateEntity(std::forward<decltype(comps)>(comps)...); },
                                                           std::move(tuple));
                                                   } });
    }
    /////////////////////////////////////

    void DestroyEntity(Entity entity) { m_destroyEntityCommands.push_back({ entity }); }

    template <typename Component>
    void AddToEntity(Entity entity, Component&& component)
    {
        using CompType = std::decay_t<Component>;
        CompType compCopy = std::forward<Component>(component);

        m_addToEntityCommandsTemplated.push_back({ entity,
            [entity, comp = std::move(compCopy)](EntityManager& entityManager) mutable { entityManager.AddToEntity(entity, std::move(comp)); } });
    }

    void AddToEntity(Entity entity, ComponentID componentId, void* componentPtr)
    {
        m_addToEntityCommandsTypeErased.push_back({ entity, componentId, componentPtr });
    }

    template <typename Component>
    void RemoveFromEntity(Entity entity)
    {
        m_addToEntityCommandsTemplated.push_back(
            { entity, [entity](EntityManager& entityManager) { entityManager.RemoveComponentFrom<Component>(entity); } });
    }

    void RemoveFromEntity(Entity entity, ComponentID componentId, void* componentPtr)
    {
        m_removeFromEntityCommandsTypeErased.push_back({ entity, componentId, componentPtr });
    }

    void ExecuteAllCommands(EntityManager& entityManager)
    {
        for (auto& command : m_createEntityCommandsTypeErased)
        {
            Entity entity = entityManager.CreateEntity(command.pendingComponents);
            if (command.returnEntity) *command.returnEntity = entity;
        }

        for (auto& command : m_createEntityCommandsTemplated)
        {
            Entity entity = command.CreateFn(entityManager);
            if (command.returnEntity) *command.returnEntity = entity;
        }

        for (auto& command : m_destroyEntityCommands) entityManager.DestroyEntity(command.entity);

        for (auto& command : m_addToEntityCommandsTemplated) command.AddFn(entityManager);

        for (auto& command : m_addToEntityCommandsTypeErased)
        {
            entityManager.AddToEntity(command.entity, command.componentID, command.componentPtr);
        }

        for (auto& command : m_removeFromEntityCommandsTemplated) command.RemoveFn(entityManager);

        for (auto& command : m_removeFromEntityCommandsTypeErased)
        {
            entityManager.RemoveComponentFrom(command.entity, command.componentID, command.componentPtr);
        }

        Clear();
    }
};
