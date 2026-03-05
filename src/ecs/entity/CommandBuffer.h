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

    struct CreateFromEntityTemplateCommand
    {
        Entity* returnEntity;
        std::vector<PendingComponent> pendingComponents;
    };

    struct CreateFromComponentCommand
    {
        Entity* returnEntity;
        std::function<Entity(EntityManager&)> CreateFn;
    };

    struct DestroyEntityCommand
    {
        Entity entity;
    };

    struct AddToEntityCommand
    {
        Entity entity;
        std::function<void(EntityManager&)> AddFn;
    };

    struct RemoveFromEntityCommand
    {
        Entity entity;
        std::function<void(EntityManager&)> RemoveFn;
    };

    std::vector<CreateFromEntityTemplateCommand> m_createFromEntityTemplateCommands;
    std::vector<CreateFromComponentCommand> m_createFromComponentCommands;
    std::vector<DestroyEntityCommand> m_destroyEntityCommands;
    std::vector<AddToEntityCommand> m_addToEntityCommands;
    std::vector<RemoveFromEntityCommand> m_removeFromEntityCommands;

    void Clear()
    {
        m_createFromEntityTemplateCommands.clear();
        m_createFromComponentCommands.clear();
        m_destroyEntityCommands.clear();
        m_addToEntityCommands.clear();
        m_removeFromEntityCommands.clear();
    }

  public:

    CommandBuffer() = default;

    // Create from EntityTemplate ///////
    std::vector<PendingComponent>& CreateEntityFromTemplate(std::vector<PendingComponent>&& components)
    {
        m_createFromEntityTemplateCommands.push_back({ nullptr, std::move(components) });
        auto& command = m_createFromEntityTemplateCommands.back();
        return command.pendingComponents;
    }

    std::vector<PendingComponent>& CreateEntityFromTemplate(Entity entity, std::vector<PendingComponent>&& components)
    {
        m_createFromEntityTemplateCommands.push_back({ &entity, std::move(components) });
        auto& command = m_createFromEntityTemplateCommands.back();
        return command.pendingComponents;
    }
    //////////////////////////////////////

    // Create from Components ////////////
    template <typename... Components>
    void CreateEntityFromComponents(Components&&... components)
    {
        auto tuple = std::make_tuple(std::forward<Components>(components)...);

        m_createFromComponentCommands.push_back({ nullptr, [tuple = std::move(tuple)](EntityManager& entityManager) mutable {
                                                     return std::apply([&](auto&&... comps)
                                                         { return entityManager.CreateEntity(std::forward<decltype(comps)>(comps)...); },
                                                         std::move(tuple));
                                                 } });
    }

    template <typename... Components>
    void CreateEntityFromComponents(Entity entity, Components&&... components)
    {
        auto tuple = std::make_tuple(std::forward<Components>(components)...);

        m_createFromComponentCommands.push_back({ &entity, [tuple = std::move(tuple)](EntityManager& entityManager) mutable {
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

        m_addToEntityCommands.push_back({ entity,
            [entity, comp = std::move(compCopy)](EntityManager& entityManager) mutable { entityManager.AddToEntity(entity, std::move(comp)); } });
    }

    template <typename Component>
    void RemoveFromEntity(Entity entity)
    {
        m_removeFromEntityCommands.push_back(
            { entity, [entity](EntityManager& entityManager) { entityManager.RemoveComponentFrom<Component>(entity); } });
    }

    void ExecuteAllCommands(EntityManager& entityManager)
    {
        for (auto& command : m_createFromEntityTemplateCommands)
        {
            Entity entity = entityManager.CreateEntity(command.pendingComponents);
            if (command.returnEntity) *command.returnEntity = entity;
        }

        for (auto& command : m_createFromComponentCommands)
        {
            Entity entity = command.CreateFn(entityManager);
            if (command.returnEntity) *command.returnEntity = entity;
        }

        for (auto& command : m_destroyEntityCommands) entityManager.DestroyEntity(command.entity);
        for (auto& command : m_addToEntityCommands) command.AddFn(entityManager);
        for (auto& command : m_removeFromEntityCommands) command.RemoveFn(entityManager);

        Clear();
    }
};
