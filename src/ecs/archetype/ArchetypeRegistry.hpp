#pragma once
#include <bitset>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "ecs/archetype/Archetype.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/common/ECSCommon.h"

struct Query
{
    ComponentSignatureMask queryMask;
    std::vector<Archetype*> matchingArchetypes;
};

class ArchetypeRegistry
{
    inline static size_t defaultArchetypeChunkCapacity = 64;

  private:

    std::vector<std::unique_ptr<Archetype>> m_archetypes;
    std::unordered_map<ComponentSignatureMask, ArchetypeId> m_signatureToArchetypIdMap;
    std::unordered_map<ComponentSignatureMask, std::unique_ptr<Query>> m_signatureToQueryMap;

    std::string MakeArchetypeName(const ComponentSignatureMask& signature)
    {
        std::string name;

        for (ComponentID id = 0; id < MaxComponents; ++id)
        {
            if (signature.test(id))
            {
                name += ComponentRegistry::GetComponentNameById(id);
                name += " ";
            }
        }

        return name;
    }

    Archetype& RegisterArchetype(const ComponentSignatureMask& signature)
    {
        ArchetypeId id = static_cast<ArchetypeId>(m_archetypes.size());
        std::string name = MakeArchetypeName(signature);

        std::unique_ptr<Archetype> newArchetype =
            std::make_unique<Archetype>(id, signature, name, defaultArchetypeChunkCapacity);

        /////////////////////////////////////////////
        newArchetype->InitializeComponentAllocators(signature);
        ////////////////////////////////////////////

        m_archetypes.emplace_back(std::move(newArchetype));
        m_signatureToArchetypIdMap.emplace(signature, id);

        for (auto& [mask, query] : m_signatureToQueryMap)
        {
            if ((mask & signature) == mask)
            {
                query->matchingArchetypes.push_back(m_archetypes.back().get());
            }
        }

        return *m_archetypes.back();
    }

  public:

    const std::vector<std::unique_ptr<Archetype>>& GetAllArchetypes() const { return m_archetypes; };
    Archetype& GetArchetypeById(ArchetypeId id) { return *m_archetypes[id]; }

    template <typename... Components>
    ComponentSignatureMask MakeSignatureMask()
    {
        ComponentSignatureMask signature;
        (signature.set(ComponentRegistry::GetComponentID<Components>()), ...);
        return signature;
    }

    Archetype& FindOrCreateArchetype(const ComponentSignatureMask& signature)
    {

        auto it = m_signatureToArchetypIdMap.find(signature);
        if (it != m_signatureToArchetypIdMap.end())
        {
            return *m_archetypes[it->second];
        }

        Archetype& newArchetype = RegisterArchetype(signature);

        return newArchetype;
    }
    template <typename... Components>
    Query& CreateQuery()
    {
        ComponentSignatureMask querySignatureMask = MakeSignatureMask<Components...>();

        // Already Exists
        auto it = m_signatureToQueryMap.find(querySignatureMask);
        if (it != m_signatureToQueryMap.end())
        {
            return *it->second;
        }

        // Create New
        std::unique_ptr<Query> newQuery = std::make_unique<Query>();
        newQuery->queryMask = querySignatureMask;

        for (auto& archetype : m_archetypes)
        {
            if ((querySignatureMask & archetype->GetComponentSignature()) == querySignatureMask)
            {
                newQuery->matchingArchetypes.push_back(archetype.get());
            }
        }

        Query& newQueryRef = *newQuery;
        m_signatureToQueryMap.emplace(querySignatureMask, std::move(newQuery));

        return newQueryRef;
    }
};
