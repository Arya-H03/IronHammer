#include "Archetype.h"
#include "ComponentRegistry.hpp"
#include "ECSCommon.h"
#include <bitset>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#pragma once

class ArchetypeRegistry
{
    inline static size_t defaultArchetypeChunkCapacity = 64;

  private:
    std::vector<std::unique_ptr<Archetype>> archetypes;
    std::unordered_map<ArchetypeComponentSignature, ArchetypeId> signatureToArchetypIdeMap;

    std::string MakeArchetypeName(const ArchetypeComponentSignature& signature)
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

    Archetype& RegisterArchetype(const ArchetypeComponentSignature& signature)
    {
        ArchetypeId id = static_cast<ArchetypeId>(archetypes.size());
        std::string name = MakeArchetypeName(signature);

        std::unique_ptr<Archetype> newArchetype =
            std::make_unique<Archetype>(id, signature, name, defaultArchetypeChunkCapacity);

        /////////////////////////////////////////////
        newArchetype->InitlizeComponentAllocators(signature);
        ////////////////////////////////////////////

        archetypes.emplace_back(std::move(newArchetype));

        signatureToArchetypIdeMap.emplace(signature, id);

        return *archetypes.back();
    }

  public:
    const std::vector<std::unique_ptr<Archetype>>& GetAllArchetypes() const { return archetypes; };
    Archetype& GetArchetypeById(ArchetypeId id) { return *archetypes[id]; }

    template <typename... Components>
    ArchetypeComponentSignature MakeArchetypeSignature()
    {
        ArchetypeComponentSignature signature;
        (signature.set(ComponentRegistry::GetComponentID<Components>()), ...);
        return signature;
    }

    Archetype& FindOrCreateArchetype(const ArchetypeComponentSignature& signature)
    {

        auto it = signatureToArchetypIdeMap.find(signature);
        if (it != signatureToArchetypIdeMap.end())
        {
            return *archetypes[it->second];
        }

        Archetype& newArchetype = RegisterArchetype(signature);

        return newArchetype;
    }
};
