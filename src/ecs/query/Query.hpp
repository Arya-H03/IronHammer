#pragma once

#include <vector>
#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"

struct QueryKey
{
    ComponentSignatureMask requiredMask {};
    ComponentSignatureMask excludedMask {};

    bool operator==(const QueryKey& other) const
    {
        return requiredMask == other.requiredMask && excludedMask == other.excludedMask;
    }
};

struct QueryKeyHash
{
    size_t operator()(const QueryKey& key) const
    {
        size_t h1 = std::hash<ComponentSignatureMask>()(key.requiredMask);
        size_t h2 = std::hash<ComponentSignatureMask>()(key.excludedMask);
        return h1 ^ (h2 << 1);
    }
};

class Query
{
  private:

    QueryKey m_key;
    std::vector<Archetype*> m_matchingArchetypes;

  public:

    Query(const QueryKey& key) : m_key(key) { }

    const QueryKey& GetKey() const { return m_key; }
    const std::vector<Archetype*>& GetMatchingArchetypes() const { return m_matchingArchetypes; }

    void TryAddArchetype(Archetype& archetype)
    {
        const auto& signature = archetype.GetComponentSignature();

        if (((m_key.requiredMask & signature) == m_key.requiredMask) && ((m_key.excludedMask & signature) == 0))
        {
            m_matchingArchetypes.push_back(&archetype);
        }
    }
};
