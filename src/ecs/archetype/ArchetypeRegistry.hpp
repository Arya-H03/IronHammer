#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "ecs/archetype/Archetype.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/query/Query.hpp"

template <typename... Component>
struct RequiredComponents
{
};

template <typename... Component>
struct ExcludedComponents
{
};

class ArchetypeRegistry
{
    inline static size_t defaultArchetypeChunkCapacity = 64;

  private:

    std::vector<std::unique_ptr<Archetype>> m_archetypes;
    std::unordered_map<ComponentSignatureMask, ArchetypeId> m_signatureToArchetypeIdMap;
    std::unordered_map<QueryKey, std::unique_ptr<Query>, QueryKeyHash> m_queryCache;

    std::string MakeArchetypeName(const ComponentSignatureMask& signature)
    {
        std::string name;

        for (ComponentID id = 0; id < MaxComponents; ++id)
        {
            if (signature.test(id))
            {
                name += ComponentRegistry::GetComponentNameById(id);
                name += "  ";
            }
        }
        return name;
    }

    Archetype& RegisterArchetype(const ComponentSignatureMask& signature)
    {
        ArchetypeId id = static_cast<ArchetypeId>(m_archetypes.size());

        std::unique_ptr<Archetype> archetype =
            std::make_unique<Archetype>(id, signature, MakeArchetypeName(signature), defaultArchetypeChunkCapacity);

        ////////////////////////////////////////////////////
        archetype->InitializeComponentAllocators(signature);
        ////////////////////////////////////////////////////

        Archetype& ref = *archetype;
        m_archetypes.emplace_back(std::move(archetype));
        m_signatureToArchetypeIdMap.emplace(signature, id);

        // Notify all cached queries
        for (auto& [key, query] : m_queryCache)
        {
            query->TryAddArchetype(ref);
        }

        return ref;
    }

    template <typename... T>
    void ProcessArgumentsForQueryKey(RequiredComponents<T...>, QueryKey& key)
    {
        (key.requiredMask.set(ComponentRegistry::GetComponentID<T>()), ...);
    }

    template <typename... T>
    void ProcessArgumentsForQueryKey(ExcludedComponents<T...>, QueryKey& key)
    {
        (key.excludedMask.set(ComponentRegistry::GetComponentID<T>()), ...);
    }

  public:

    const std::vector<std::unique_ptr<Archetype>>& GetAllArchetypes() const { return m_archetypes; }
    Archetype& GetArchetypeById(ArchetypeId id) { return *m_archetypes[id]; }

    Archetype& GetArchetype(const ComponentSignatureMask& signature)
    {
        auto it = m_signatureToArchetypeIdMap.find(signature);
        if (it != m_signatureToArchetypeIdMap.end())
        {
            return *m_archetypes[it->second];
        }

        return RegisterArchetype(signature);
    }

    template <typename... Components>
    ComponentSignatureMask MakeSignatureMask()
    {
        ComponentSignatureMask signature;
        (signature.set(ComponentRegistry::GetComponentID<Components>()), ...);
        return signature;
    }

    template <typename... QueryGroup>
    Query* GetOrCreateQuery()
    {
        QueryKey key;
        (ProcessArgumentsForQueryKey(QueryGroup {}, key), ...);

        auto it = m_queryCache.find(key);
        if (it != m_queryCache.end())
        {
            return it->second.get();
        }

        std::unique_ptr<Query> query = std::make_unique<Query>(key);

        // Let query evaluate existing archetypes
        for (auto& archetype : m_archetypes)
        {
            query->TryAddArchetype(*archetype);
        }

        Query* ptr = query.get();
        m_queryCache.emplace(key, std::move(query));

        return ptr;
    }
};
