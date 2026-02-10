#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <stdint.h>
#include <tuple>
#include <vector>
#include <imgui.h>
#include "ComponentRegistry.hpp"
#include "ECSCommon.h"
#include "BaseArchetype.h"
#include "SlabAllocator.hpp"

// Address having new entities being added that the back chunk always.

template <size_t ChunkSize, typename... Components>
class Archetype final : public BaseArchetype
{
    static_assert(ChunkSize > 0);

  private:
    using AllocatorTuple = std::tuple<SlabAllocator<Components, ChunkSize>...>;
    AllocatorTuple allocatorTuple;

    struct ArchetypeChunk
    {
        std::tuple<Components*...> componentArrayTuple;
        EntityID entityIds[ChunkSize]{};
        size_t count = 0;
        bool IsFull() const
        {
            return count == ChunkSize;
        }

        ArchetypeChunk()
        {
            std::fill(std::begin(entityIds), std::end(entityIds), InvalidEntityID);
        }
    };

    std::vector<ArchetypeChunk> archetypeChunks;

    ArchetypeChunk& CreateArchetypeChunk()
    {
        ArchetypeChunk chunk;

        chunk.componentArrayTuple =
            std::make_tuple(std::get<SlabAllocator<Components, ChunkSize>>(allocatorTuple).AllocateArray()...);

        archetypeChunks.push_back(chunk);
        return archetypeChunks.back();
    }

    template <typename Tuple, size_t... I>
    EntityArchetypeLocation AddEntityImplementation(uint32_t entityId, Tuple& componentValues, std::index_sequence<I...>)
    {
        if (archetypeChunks.empty() || archetypeChunks.back().IsFull()) CreateArchetypeChunk();

        ArchetypeChunk& chunk = archetypeChunks.back();
        const size_t index = chunk.count;

        // Move all values for the components
        ((std::get<Components*>(chunk.componentArrayTuple)[index] = std::move(std::get<I>(componentValues))), ...);

        ++chunk.count;
        uint32_t chunkIndex = static_cast<uint32_t>(archetypeChunks.size() - 1);
        uint32_t indexInChunk = static_cast<uint32_t>(chunk.count - 1);
        chunk.entityIds[indexInChunk] = entityId;

        return EntityArchetypeLocation{archetypeId, chunkIndex, indexInChunk};
    }

    template <typename TComponent>
    static void DrawComponentDebugGUI(const TComponent& component)
    {
        ImGui::Text(
            "%s: %s", ComponentRegistry::GetComponentNameByType(component).c_str(), ComponentRegistry::GetComponentInfo(component).c_str());
    }

  public:
    EntityArchetypeLocation AddEntity(uint32_t entityId, void* componentTuple) override
    {
        return AddEntityImplementation(
            entityId, *static_cast<std::tuple<Components...>*>(componentTuple), std::index_sequence_for<Components...>{});
    }

    void RemoveEntity(uint32_t entityId, uint32_t chunkIndex, uint32_t indexInChunk) override
    {
        ArchetypeChunk& chunk = archetypeChunks[chunkIndex];
        const size_t lastIndexInChunk = chunk.count - 1;

        if (indexInChunk != lastIndexInChunk)
        {
            ((std::get<Components*>(chunk.componentArrayTuple)[indexInChunk] =
                  std::move(std::get<Components*>(chunk.componentArrayTuple)[lastIndexInChunk])),
             ...);
        }

        chunk.entityIds[indexInChunk] = InvalidEntityID;
        --chunk.count;
    }

    void DrawDebugGUI() const override
    {
        int entityTreeID = 0;
        for (int currentChunkIndex = 0; currentChunkIndex < archetypeChunks.size(); ++currentChunkIndex)
        {
            const ArchetypeChunk& currentChunk = archetypeChunks[currentChunkIndex];
            for (int currentIndexInCunk = 0; currentIndexInCunk < currentChunk.count; ++currentIndexInCunk)
            {
                ImGui::PushID(entityTreeID);
                if (ImGui::TreeNode("", "Entity: %d", currentChunk.entityIds[currentIndexInCunk]))
                {
                    ((DrawComponentDebugGUI(std::get<Components*>(currentChunk.componentArrayTuple)[currentIndexInCunk])),
                     ...);

                    ImGui::TreePop();
                }
                ImGui::PopID();
                ++entityTreeID;

                ImGui::Separator();
            }
        }
    }
};
