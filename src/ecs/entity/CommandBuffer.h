#pragma once

#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"

#include <algorithm>
#include <fcntl.h>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Future Optimization:
// Could Implement a Small Buffer Optimization for
// Template commands that store component data via void*
// Have a char inlineBuffer [64] allgined to alignas(std::max_align_t)
// Ensure Move constructor then moves this buffer upon vector reallocation
// Delete Copy constructor

class CommandBuffer
{
  private:
    struct CreateEntityCommandTypeErased
    {
        Entity entity;

        std::vector<PendingComponent> pendingComponents;
    };

    struct CreateEntityCommandTemplated
    {
        Entity entity;
        void* data;

        void (*ExecuteFn)(EntityManager&, Entity, void*);
        void (*DestroyFn)(void*);
    };

    struct DestroyEntityCommand
    {
        Entity entity;
    };

    struct AddToEntityCommandTemplated
    {
        Entity entity;
        void* data;

        void (*ExecuteFn)(EntityManager&, Entity, void*);
        void (*DestroyFn)(void*);
    };

    struct AddToEntityCommandTypeErased
    {
        Entity entity;
        ComponentId componentID;
        void* componentPtr;
    };

    struct RemoveFromEntityCommandTemplated
    {
        Entity entity;

        void (*ExecuteFn)(EntityManager&, Entity entity);
    };

    struct RemoveFromEntityCommandTypeErased
    {
        Entity entity;
        ComponentId componentID;
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
    std::pair<Entity, std::vector<PendingComponent>&> CreateEntityFromMold(EntityManager& entityManager,
                                                                           std::vector<PendingComponent>&& components)
    {
        Entity entity = entityManager.GenerateEntity();
        m_createEntityCommandsTypeErased.push_back({entity, std::move(components)});
        auto& command = m_createEntityCommandsTypeErased.back();
        return {entity, command.pendingComponents};
    }
    //////////////////////////////////////

    // Create from Components ////////////
    template <typename... Components>
    void CreateEntityFromComponentsNoReturn(EntityManager& entityManager, Components&&... components)
    {
        Entity entity = entityManager.GenerateEntity();
        using ComponentTuple = std::tuple<std::decay_t<Components>...>;

        ComponentTuple* componentTupleData = new ComponentTuple(std::forward<Components>(components)...);

        m_createEntityCommandsTemplated.push_back(
            {entity, componentTupleData,
             // Execute Function
             [](EntityManager& entityManager, Entity entity, void* data)
             {
                 ComponentTuple* componentTuplePtr = static_cast<ComponentTuple*>(data);
                 std::apply([&](auto&&... comps) { entityManager.CreateEntity(entity, std::forward<decltype(comps)>(comps)...); },
                            std::move(*componentTuplePtr));
             },
             // Destroy Function
             [](void* data) { delete static_cast<ComponentTuple*>(data); }});
    }

    template <typename... Components>
    Entity CreateEntityFromComponentsWithReturn(EntityManager& entityManager, Components&&... components)
    {
        Entity entity = entityManager.GenerateEntity();
        using ComponentTuple = std::tuple<std::decay_t<Components>...>;

        ComponentTuple* componentTupleData = new ComponentTuple(std::forward<Components>(components)...);

        m_createEntityCommandsTemplated.push_back(
            {entity, componentTupleData,
             // Execute Function
             [](EntityManager& entityManager, Entity entity, void* data)
             {
                 ComponentTuple* componentTuplePtr = static_cast<ComponentTuple*>(data);
                 std::apply([&](auto&&... comps) { entityManager.CreateEntity(entity, std::forward<decltype(comps)>(comps)...); },
                            std::move(*componentTuplePtr));
             },
             // Destroy Function
             [](void* data) { delete static_cast<ComponentTuple*>(data); }});

        return entity;
    }
    /////////////////////////////////////

    void DestroyEntity(Entity entity)
    {
        m_destroyEntityCommands.push_back({entity});
    }

    template <typename Component>
    void AddToEntity(Entity entity, Component&& component)
    {
        using CompType = std::decay_t<Component>;
        CompType* compData = new CompType(std::forward<Component>(component));

        m_addToEntityCommandsTemplated.push_back({entity, compData,
                                                  // Execute Function
                                                  [](EntityManager& entityManager, Entity entity, void* data)
                                                  {
                                                      CompType* comp = static_cast<CompType*>(data);
                                                      entityManager.AddToEntity(entity, std::forward<CompType>(*comp));
                                                  },
                                                  // Destroy Function
                                                  [](void* data) { delete static_cast<CompType*>(data); }});
    }

    void AddToEntity(Entity entity, ComponentId componentId, void* componentPtr)
    {
        m_addToEntityCommandsTypeErased.push_back({entity, componentId, componentPtr});
    }

    template <typename Component>
    void RemoveFromEntity(Entity entity)
    {
        m_removeFromEntityCommandsTemplated.push_back({entity,
                                                       // Execute Function
                                                       [](EntityManager& entityManager, Entity entity)
                                                       { entityManager.RemoveComponentFrom<Component>(entity); }});
    }

    void RemoveFromEntity(Entity entity, ComponentId componentId)
    {
        m_removeFromEntityCommandsTypeErased.push_back({entity, componentId});
    }

    void ExecuteAllCommands(EntityManager& entityManager)
    {
        for (auto& command : m_createEntityCommandsTypeErased)
        {
            entityManager.CreateEntity(command.entity, command.pendingComponents);
        }

        for (auto& command : m_createEntityCommandsTemplated)
        {
            command.ExecuteFn(entityManager, command.entity, command.data);
            command.DestroyFn(command.data);
        }

        for (auto& command : m_destroyEntityCommands)
            entityManager.DestroyEntity(command.entity);

        for (auto& command : m_addToEntityCommandsTemplated)
        {
            command.ExecuteFn(entityManager, command.entity, command.data);
            command.DestroyFn(command.data);
        }

        for (auto& command : m_addToEntityCommandsTypeErased)
        {
            entityManager.AddToEntity(command.entity, command.componentID, command.componentPtr);
        }

        for (auto& command : m_removeFromEntityCommandsTemplated)
        {
            command.ExecuteFn(entityManager, command.entity);
        }

        for (auto& command : m_removeFromEntityCommandsTypeErased)
        {
            entityManager.RemoveComponentFrom(command.entity, command.componentID);
        }

        Clear();
    }
};
