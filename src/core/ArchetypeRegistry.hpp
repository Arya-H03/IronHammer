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

    template <typename... Components>
    ArchetypeComponentSignature MakeArchetypeSignature()
    {
        ArchetypeComponentSignature signature;
        (signature.set(ComponentRegistry::GetComponentID<Components>()), ...);
        return signature;
    }

    template <typename... Components>
    std::string MakeArchetypeName()
    {
        std::string archetypeName;
        ((archetypeName +=
          std::string(ComponentRegistry::GetComponentNameById(ComponentRegistry::GetComponentID<Components>())) + " "),
         ...);
        return archetypeName;
    }

    template <typename... Components>
    Archetype& RegisterArchetype()
    {
        ArchetypeId id = static_cast<ArchetypeId>(archetypes.size());
        ArchetypeComponentSignature signature = MakeArchetypeSignature<Components...>();
        std::string name = MakeArchetypeName<Components...>();

        std::unique_ptr<Archetype> newArchetype =
            std::make_unique<Archetype>(id, signature, name, defaultArchetypeChunkCapacity);

        /////////////////////////////////////////////
        newArchetype->InitlizeComponentAllocators<Components...>();
        ////////////////////////////////////////////

        archetypes.emplace_back(std::move(newArchetype));

        signatureToArchetypIdeMap.emplace(signature, id);

        return *archetypes.back();
    }

  public:
    const std::vector<std::unique_ptr<Archetype>>& GetAllArchetypes() const { return archetypes; };
    Archetype& GetArchetypeById(ArchetypeId id) { return *archetypes[id]; }

    template <typename... Components>
    Archetype& FindOrCreateArchetype()
    {
        ArchetypeComponentSignature signature = MakeArchetypeSignature<Components...>();

        auto it = signatureToArchetypIdeMap.find(signature);
        if (it != signatureToArchetypIdeMap.end())
        {
            return *archetypes[it->second];
        }

        Archetype& newArchetype = RegisterArchetype<Components...>();

        return newArchetype;
    }
};
