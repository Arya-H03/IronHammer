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
#include "ecs/system/SystemRegistry.hpp"


class ArchetypeRegistry
{
    inline static size_t defaultArchetypeChunkCapacity = 64;

  private:
    std::vector<std::unique_ptr<Archetype>> m_archetypes;
    std::unordered_map<ComponentSignatureMask, ArchetypeId> m_signatureToArchetypIdeMap;
    SystemRegistry& m_systemRegistry;

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
        m_signatureToArchetypIdeMap.emplace(signature, id);

        for(auto& system : m_systemRegistry.GetSystems())
        {
            system->TryAddMatchingArchetype(m_archetypes.back().get());
        }

        return *m_archetypes.back();
    }

  public:
    const std::vector<std::unique_ptr<Archetype>>& GetAllArchetypes() const { return m_archetypes; };
    Archetype& GetArchetypeById(ArchetypeId id) { return *m_archetypes[id]; }

    ArchetypeRegistry(SystemRegistry& systemRegistry):m_systemRegistry(systemRegistry){}

    template <typename... Components>
    ComponentSignatureMask MakeArchetypeSignature()
    {
        ComponentSignatureMask signature;
        (signature.set(ComponentRegistry::GetComponentID<Components>()), ...);
        return signature;
    }

    Archetype& FindOrCreateArchetype(const ComponentSignatureMask& signature)
    {

        auto it = m_signatureToArchetypIdeMap.find(signature);
        if (it != m_signatureToArchetypIdeMap.end())
        {
            return *m_archetypes[it->second];
        }

        Archetype& newArchetype = RegisterArchetype(signature);

        return newArchetype;
    }
};
