#pragma once
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityManager.hpp"
#include <algorithm>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

class CommandBuffer
{
  private:

    struct ICommand
    {
        virtual ~ICommand() = default;
        virtual void Execute(EntityManager& entityManager) = 0;
    };

    template <typename... Components>
    struct CreateEntityCommandWithReturn : public ICommand
    {
        Entity& m_entity;
        std::tuple<Components...> m_componentTuple;
        CreateEntityCommandWithReturn(Entity& entity, Components&&... components)
            : m_entity(entity), m_componentTuple(std::forward<Components>(components)...)
        {
        }
        void Execute(EntityManager& entityManager) override
        {
            m_entity = std::apply(
                [&](auto&&... comps) { return entityManager.CreateEntity(std::forward<decltype(comps)>(comps)...); }, std::move(m_componentTuple));
        }
    };

    template <typename... Components>
    struct CreateEntityCommandNoReturn : public ICommand
    {
        std::tuple<Components...> m_componentTuple;
        CreateEntityCommandNoReturn(Components&&... components) : m_componentTuple(std::forward<Components>(components)...) { }
        void Execute(EntityManager& entityManager) override
        {
            std::apply(
                [&](auto&&... comps) { return entityManager.CreateEntity(std::forward<decltype(comps)>(comps)...); }, std::move(m_componentTuple));
        }
    };

    struct DestroyEntityCommand : public ICommand
    {
        Entity m_entity;
        DestroyEntityCommand(Entity entity) : m_entity(entity) { }
        void Execute(EntityManager& entityManager) override { entityManager.DestroyEntity(m_entity); }
    };

    template <typename Component>
    struct AddToEntityCommand : public ICommand
    {
        Component m_component;
        Entity m_entity;
        AddToEntityCommand(Entity entity, Component&& component) : m_component(component), m_entity(entity) { }

        void Execute(EntityManager& entityManager) override { entityManager.AddToEntity(m_entity, std::forward<Component>(m_component)); }
    };

    template <typename Component>
    struct RemoveFromEntityCommand : public ICommand
    {
        Entity m_entity;
        RemoveFromEntityCommand(Entity entity) : m_entity(entity) { }
        void Execute(EntityManager& entityManager) override { entityManager.RemoveComponentFrom<Component>(m_entity); }
    };

    std::vector<std::unique_ptr<ICommand>> m_commands;
    EntityManager& m_entityManager;

  public:

    CommandBuffer(EntityManager& entityManager) : m_entityManager(entityManager) { }

    void ExecuteAllCommands()
    {
        for (auto& command : m_commands)
        {
            command->Execute(m_entityManager);
        }

        m_commands.clear();
    }

    template <typename... Components>
    void CreateEntity(Entity& entity, Components&&... components)
    {
        m_commands.push_back(std::make_unique<CreateEntityCommandWithReturn<Components...>>(entity, std::forward<Components>(components)...));
    }

    template <typename... Components>
    void CreateEntity(Components&&... components)
    {
        m_commands.push_back(std::make_unique<CreateEntityCommandNoReturn<Components...>>(std::forward<Components>(components)...));
    }

    void DestroyEntity(Entity entity) { m_commands.push_back(std::make_unique<DestroyEntityCommand>(entity)); }

    template <typename Component>
    void AddToEntity(Entity entity, Component&& component)
    {
        m_commands.push_back(std::make_unique<AddToEntityCommand<std::decay_t<Component>>>(entity, std::forward<Component>(component)));
    }

    template <typename Component>
    void RemoveFromEntity(Entity entity)
    {
        m_commands.push_back(std::make_unique<RemoveFromEntityCommand<Component>>(entity));
    }
};
