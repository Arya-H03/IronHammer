#pragma once
#include <functional>
#include <imgui.h>
#include <string>
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/archetype/Archetype.h"
#include "ecs/entity/EntityCommands.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "core/utils/Colors.h"
#include "ecs/entity/EntityInspector.hpp"

class ArchetypeDebugger
{
  private:

    ArchetypeRegistry& m_archetypeRegistry;
    CommandBuffer& m_commandBuffer;
    EntityInspector& m_entityInspector;

    void DrawIndividualArchetypeGUI(Archetype& archetype, const std::function<void(Entity)>& deleteEntityCallBack) const
    {
        ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "Capacity: %zu", archetype.m_chunkCapacity * archetype.m_chunks.size());
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(Colors::HazardYellow_ImGui, "Size: %zu", archetype.m_totalSize);
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(Colors::ColdSteelBlue_ImGui, "Chunk: %zu", archetype.m_chunks.size());
        ImGui::SameLine();
        ImGui::Text("|");
        ImGui::SameLine();
        ImGui::TextColored(Colors::RustRed_ImGui, "Chunk Capacity: %zu", archetype.m_chunkCapacity);

        ImGui::Separator();

        int totalEntities = static_cast<int>(archetype.m_totalSize);

        ImGuiListClipper clipper;
        clipper.Begin(totalEntities);

        while (clipper.Step())
        {
            for (int linearIndex = clipper.DisplayStart; linearIndex < clipper.DisplayEnd; ++linearIndex)
            {
                int remaining = linearIndex;
                Entity currentEntity {};

                for (int chunkIndex = 0; chunkIndex < archetype.m_chunks.size(); ++chunkIndex)
                {
                    const auto& chunk = archetype.m_chunks[chunkIndex];

                    if (remaining < chunk.size)
                    {
                        currentEntity = chunk.entities[remaining];
                        break;
                    }

                    remaining -= chunk.size;
                }

                ImGui::PushID(currentEntity.id);

                if (ImGui::Button("D", ImVec2(20, 20)))
                {
                    deleteEntityCallBack(currentEntity);
                }

                ImGui::SameLine();
                bool isSelected = (m_entityInspector.GetCurrentInspectorEntity() == currentEntity);
                if (ImGui::Selectable("Entity", isSelected))
                {
                    m_entityInspector.SetCurrentInspectorEntity(currentEntity);
                }
                ImGui::SameLine();
                ImGui::Text("%u", currentEntity.id);

                ImGui::PopID();
                ImGui::Separator();
            }
        }
    }

  public:

    ArchetypeDebugger(
        ArchetypeRegistry& archetypeRegistry, CommandBuffer& commandBuffer, EntityInspector& entityInspector)
        :  m_archetypeRegistry(archetypeRegistry), m_commandBuffer(commandBuffer), m_entityInspector(entityInspector)
    {
    }

    void DrawArchetypeGuiTab() const
    {
        Log_Error("w");
        for (auto& archetype : m_archetypeRegistry.GetAllArchetypes())
        {
            std::string nodeTitle = "Arch " + std::to_string(archetype->GetArchetypeId()) + ": " + archetype->GetArchetypeName();

            if (ImGui::CollapsingHeader(nodeTitle.c_str()))
            {
                DrawIndividualArchetypeGUI(*archetype, [&](Entity entity) { m_commandBuffer.DestroyEntity(entity); });
            }
        }
    }
};
