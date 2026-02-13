#pragma once
#include <cstddef>
#include <functional>
#include "ComponentRegistry.hpp"
#include "ECSCommon.h"
#include "Archetype.h"
#include "EntityManager.hpp"
#include <imgui.h>

class ArchetypeDebugger
{
  private:
    void DrawIndividualArchetypeGUI(const Archetype& archetype,
                                    const std::function<void(Entity)>& deleteEntityCallBack) const
    {
        int entityTreeID = 0;
        ImGui::TextColored(
            ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "Capacity: %zu", archetype.m_chunkCapacity * archetype.m_chunks.size());
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.15f, 0.15f, 1.0f), "Size: %zu", archetype.m_totalSize);
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.4f, 0.4f, 0.4f, 1.0f), "Chunk: %zu", archetype.m_chunks.size());
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.85f, 0.55f, 0.2f, 1.0f), "Chunk Capacity: %zu", archetype.m_chunkCapacity);

        ImGui::Separator();
        for (int currentChunkIndex = 0; currentChunkIndex < archetype.m_chunks.size(); ++currentChunkIndex)
        {
            const auto& currentChunk = archetype.m_chunks[currentChunkIndex];
            for (int currentIndexInCunk = 0; currentIndexInCunk < currentChunk.size; ++currentIndexInCunk)
            {
                ImGui::PushID(entityTreeID);

                if (ImGui::Button("D", ImVec2(20, 20)))
                {
                    deleteEntityCallBack(currentChunk.entities[currentIndexInCunk]);
                }
                ImGui::SameLine();

                if (ImGui::TreeNode("", "Entity: %d", currentChunk.entities[currentIndexInCunk].id))
                {
                    for (size_t i = 0; i < archetype.m_allocators.size(); ++i)
                    {
                        void* rawBlock = currentChunk.components[i];
                        size_t componentSize = archetype.m_allocators[i].GetDataSize();
                        char* base = static_cast<char*>(rawBlock);
                        void* componentPtr = base + (componentSize * currentChunkIndex);
                        ComponentID id = archetype.m_densIds[i];
                        const ComponentInfo& info = ComponentRegistry::GetComponentInfoById(id);
                        info.DrawGuiFn(componentPtr);
                    }

                    ImGui::TreePop();
                }

                ++entityTreeID;

                ImGui::PopID();
                ImGui::Separator();
            }
        }
    }

  public:
    void DrawArchetypeGuiTab(EntityManager& entityManager)
    {
        if (ImGui::BeginTabItem("Archetypes"))
        {
            int archetypeTreeNodeId = 0;
            for (auto& archetype : entityManager.archetypeRegistry.GetAllArchetypes())
            {
                ImGui::PushID(archetypeTreeNodeId);

                std::string nodeTitle =
                    "Arch " + std::to_string(archetype->GetArchetypeId()) + ": " + archetype->GetArchetypeName();

                if (ImGui::TreeNode(nodeTitle.c_str()))
                {
                    DrawIndividualArchetypeGUI(*archetype, [&](Entity entity) { entityManager.DeleteEntity(entity); });
                    ImGui::TreePop();
                }
                ImGui::PopID();
                ++archetypeTreeNodeId;
            }
            ImGui::EndTabItem();
        }
    }
};
