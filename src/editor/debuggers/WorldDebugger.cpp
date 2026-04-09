#include "editor/debuggers/WorldDebugger.h"

#include "ecs/World.h"
#include "ecs/archetype/Archetype.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "editor/entityInspector/EntityInspector.h"
#include "engine/Engine.h"

WorldDebugger::WorldDebugger()
{
    m_displayMode = EngineMode::Both;
}

void WorldDebugger::DrawTab(DebugTabContext& context)
{

    if (ImGui::BeginTabItem("World"))
    {
        DrawWorldGuiTab(context.worldPtr, context.entityInspectorPtr);
        ImGui::EndTabItem();
    }
}
void WorldDebugger::DrawIndividualArchetypeGUI(World* worldPtr, Archetype* archetypePtr, Inspector* inspectorPtr) const
{
    ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.75f, 1.0f), "Capacity: %zu", archetypePtr->m_chunkCapacity * archetypePtr->m_chunks.size());
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::TextColored(Colors::HazardYellow_ImGui, "Size: %zu", archetypePtr->m_totalSize);
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::TextColored(Colors::ColdSteelBlue_ImGui, "Chunk: %zu", archetypePtr->m_chunks.size());
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    ImGui::TextColored(Colors::RustRed_ImGui, "Chunk Capacity: %zu", archetypePtr->m_chunkCapacity);

    ImGui::Separator();

    int totalEntities = static_cast<int>(archetypePtr->m_totalSize);

    ImGuiListClipper clipper;
    clipper.Begin(totalEntities);

    while (clipper.Step())
    {
        for (int linearIndex = clipper.DisplayStart; linearIndex < clipper.DisplayEnd; ++linearIndex)
        {
            int remaining = linearIndex;
            Entity currentEntity{};

            for (int chunkIndex = 0; chunkIndex < archetypePtr->m_chunks.size(); ++chunkIndex)
            {
                const auto& chunk = archetypePtr->m_chunks[chunkIndex];

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
                worldPtr->DestroyEntity(currentEntity);
            }

            ImGui::SameLine();
            bool isSelected = (inspectorPtr->GetCurrentInspectorEntity() == currentEntity);
            if (ImGui::Selectable("Entity", isSelected))
            {
                inspectorPtr->InspectLiveEntity(currentEntity, worldPtr->GetEntityManager());
            }
            ImGui::SameLine();
            ImGui::Text("%u", currentEntity.id);

            ImGui::PopID();
            ImGui::Separator();
        }
    }
}

void WorldDebugger::DrawWorldGuiTab(World* worldPtr, Inspector* inspectorPtr) const
{
    if (!worldPtr || !inspectorPtr)
    {
        return;
    }

    ArchetypeRegistry& archetypeRegistry = worldPtr->GetArchetypeRegistry();

    for (auto& archetype : archetypeRegistry.GetAllArchetypes())
    {

        std::string nodeTitle = "Arch " + std::to_string(archetype->GetArchetypeId()) + ": " + archetype->GetArchetypeName();

        if (ImGui::CollapsingHeader(nodeTitle.c_str()))
        {
            DrawIndividualArchetypeGUI(worldPtr, archetype.get(), inspectorPtr);
        }
    }
}
