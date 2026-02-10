#include "Archetype.hpp"
#include "BaseArchetype.h"
#include "ComponentRegistry.hpp"
#include "ECSCommon.h"
#include <bitset>
#include <string>
#include <unordered_map>
#pragma once

using ArchetypeComponentSignature = std::bitset<MaxComponents>;

class ArchetypeRegistry
{
  private:
    std::vector<BaseArchetype*> archetypePtrs;
    std::unordered_map<ArchetypeComponentSignature, BaseArchetype*> archetypeSignatureToPtrMap;
    std::unordered_map<ArchetypeComponentSignature, std::string> archetypeSignatureToNameMap;

    template <typename... Components>
    ArchetypeComponentSignature MakeASignature()
    {
        ArchetypeComponentSignature signature;
        std::string archetypeName;
        (signature.set(ComponentRegistry::GetComponentID<Components>()), ...);
        ((archetypeName += ComponentRegistry::GetComponentNameById(ComponentRegistry::GetComponentID<Components>()) + " "),
         ...);

        archetypeSignatureToNameMap[signature] = archetypeName;
        return signature;
    }

    template <typename ArchetypeT>
    void RegisterArchetype(ArchetypeT* newArchetypePtr, ArchetypeComponentSignature signature)
    {
        ArchetypeID id = static_cast<ArchetypeID>(archetypePtrs.size());
        newArchetypePtr->archetypeId = id;
        archetypePtrs.push_back(newArchetypePtr);
        archetypeSignatureToPtrMap[signature] = newArchetypePtr;
    }

  public:

    const std::unordered_map<ArchetypeComponentSignature, BaseArchetype*>& GetSignatureToPtrMap() const
    {
        return archetypeSignatureToPtrMap;
    }

    const std::unordered_map<ArchetypeComponentSignature, std::string>& GetSignatureToNameMap() const
    {
        return archetypeSignatureToNameMap;
    }

    BaseArchetype* GetArchetypeById(ArchetypeID id)
    {
        return archetypePtrs[id];
    }

    template <typename... Components>
    BaseArchetype* FindOrCreateArchetype()
    {
        ArchetypeComponentSignature signature = MakeASignature<Components...>();
        if (archetypeSignatureToPtrMap.contains(signature))
        {
            return archetypeSignatureToPtrMap[signature];
        }
        else
        {
            Archetype<64, Components...>* newArchetype = new Archetype<64, Components...>();
            RegisterArchetype(newArchetype, signature);
            return archetypePtrs.back();
        }
    }
};
