#pragma once
#include <imgui.h>
#include <iostream>
#include <tuple>
#include <vector>
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/archetype/BaseTemplatedArchetype.h"
#include "core/memory/TemplatedSlabAllocator.hpp"

// Address having new entities being added that the back chunk always.

template <size_t ChunkSize, typename... Components>
class TemplatedArchetype final : public BaseTemplatedArchetype
{
    static_assert(ChunkSize > 0);

  private:

    using AllocatorTuple = std::tuple<TemplatedSlabAllocator<Components, ChunkSize>...>;
    AllocatorTuple allocatorTuple;

    struct ArchetypeChunk
    {
        std::tuple<Components*...> componentArrayTuple;
        Entity entities[ChunkSize];
        size_t count = 0;

        bool IsFull() const { return count == ChunkSize; }
    };

    std::vector<ArchetypeChunk> archetypeChunks;

    ArchetypeChunk& CreateArchetypeChunk()
    {
        ArchetypeChunk chunk;

        chunk.componentArrayTuple =
            std::make_tuple(std::get<TemplatedSlabAllocator<Components, ChunkSize>>(allocatorTuple).AllocateArray()...);

        archetypeChunks.push_back(chunk);
        return archetypeChunks.back();
    }

    template <typename Tuple, size_t... I>
    EntityStorageLocation AddEntityImplementation(Entity entity, Tuple& componentValues, std::index_sequence<I...>)
    {
        if (archetypeChunks.empty() || archetypeChunks.back().IsFull()) CreateArchetypeChunk();

        ArchetypeChunk& chunk = archetypeChunks.back();
        const size_t index = chunk.count;

        // Move all values for the components
        ((std::get<Components*>(chunk.componentArrayTuple)[index] = std::move(std::get<I>(componentValues))), ...);

        ++chunk.count;
        ++totalCount;
        uint32_t chunkIndex = static_cast<uint32_t>(archetypeChunks.size() - 1);
        uint32_t indexInChunk = static_cast<uint32_t>(chunk.count - 1);
        chunk.entities[indexInChunk] = entity;

        return EntityStorageLocation { archetypeId, chunkIndex, indexInChunk };
    }

    template <typename TComponent>
    static void DrawComponentDebugGUI(const TComponent& component)
    {
        ImGui::Text("%s: %s",
            ComponentRegistry::GetComponentNameByType(component).c_str(),
            ComponentRegistry::GetComponentDescription(component).c_str());
    }

  public:

    EntityStorageLocation AddEntity(Entity entity, void* componentTuple) override
    {
        return AddEntityImplementation(entity,
            *static_cast<std::tuple<Components...>*>(componentTuple),
            std::index_sequence_for<Components...> {});
    }

    std::pair<Entity, EntityStorageLocation> RemoveEntity(
        Entity entity, uint32_t chunkIndex, uint32_t indexInChunk) override
    {
        ArchetypeChunk& chunk = archetypeChunks[chunkIndex];
        const size_t lastIndexInChunk = chunk.count - 1;

        if (indexInChunk != lastIndexInChunk)
        {
            ((std::get<Components*>(chunk.componentArrayTuple)[indexInChunk] =
                     std::move(std::get<Components*>(chunk.componentArrayTuple)[lastIndexInChunk])),
                ...);

            chunk.entities[indexInChunk] = chunk.entities[lastIndexInChunk];
        }

        --chunk.count;
        --totalCount;

        return { chunk.entities[indexInChunk], { archetypeId, chunkIndex, indexInChunk } };
    }

    void DrawArchetypeGUI(const std::function<void(Entity)>& deleteEntityCallBack) const override
    {

        int entityTreeID = 0;
        ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "Capacity: %zu", ChunkSize * archetypeChunks.size());
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.15f, 0.15f, 1.0f), "Size: %zu", totalCount);
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Chunk: %zu", archetypeChunks.size());
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.85f, 0.55f, 0.2f, 1.0f), "Chunk Size: %zu", ChunkSize);

        ImGui::Separator();
        for (int currentChunkIndex = 0; currentChunkIndex < archetypeChunks.size(); ++currentChunkIndex)
        {
            const auto& currentChunk = archetypeChunks[currentChunkIndex];
            for (int currentIndexInCunk = 0; currentIndexInCunk < currentChunk.count; ++currentIndexInCunk)
            {
                ImGui::PushID(entityTreeID);

                if (ImGui::Button("D", ImVec2(20, 20)))
                {
                    std::cerr << currentChunk.entities[currentIndexInCunk].id;
                    deleteEntityCallBack(currentChunk.entities[currentIndexInCunk]);
                }
                ImGui::SameLine();

                if (ImGui::TreeNode("", "Entity: %d", currentChunk.entities[currentIndexInCunk].id))
                {

                    ((DrawComponentDebugGUI(
                         std::get<Components*>(currentChunk.componentArrayTuple)[currentIndexInCunk])),
                        ...);

                    ImGui::TreePop();
                }

                ++entityTreeID;

                ImGui::PopID();
                ImGui::Separator();
            }
        }
    }
};
