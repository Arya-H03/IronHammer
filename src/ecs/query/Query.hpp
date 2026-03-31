#pragma once

#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"

#include <tuple>
#include <vector>

struct QueryKey
{
    ComponentSignatureMask requiredMask{};
    ComponentSignatureMask excludedMask{};

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
    QueryKey                m_key;
    std::vector<Archetype*> m_matchingArchetypes;

public:
    Query(const QueryKey& key) : m_key(key) {}

    const QueryKey& GetKey() const { return m_key; }

    void TryAddArchetype(Archetype& archetype)
    {
        const auto& signature = archetype.GetComponentSignature();

        if (((m_key.requiredMask & signature) == m_key.requiredMask) && ((m_key.excludedMask & signature) == 0)) {
            m_matchingArchetypes.push_back(&archetype);
        }
    }

    template <typename... Components, typename Func>
    void ForEach(Func&& func)
    {
        for (auto& archetype : m_matchingArchetypes) {
            for (auto& chunk : archetype->GetChunks()) {
                auto componentRows = std::make_tuple(chunk.GetComponentRow<Components>()...);
                for (size_t i = 0; i < chunk.size; ++i) {
                    std::apply([&](auto*... rows) { func(rows[i]...); }, componentRows);
                }
            }
        }
    }

    template <typename... Components, typename Func>
    void ForEachWithEntity(Func&& func)
    {
        for (auto& archetype : m_matchingArchetypes) {
            for (auto& chunk : archetype->GetChunks()) {
                auto componentRows = std::make_tuple(chunk.GetComponentRow<Components>()...);
                for (size_t i = 0; i < chunk.size; ++i) {
                    std::apply([&](auto*... rows) { func(chunk.entities[i], rows[i]...); }, componentRows);
                }
            }
        }
    }
};
