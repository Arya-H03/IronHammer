#include "EntityInspector.h"

#include "core/CoreComponents.hpp"
#include "core/utils/Colors.h"
#include "core/utils/Debug.h"
#include "ecs/World.hpp"
#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"
#include "editor/entityInspector/EntityInspectorHelper.h"
#include "engine/Engine.h"
#include "imgui.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

const Entity Inspector::GetCurrentInspectorEntity() const
{
    return m_currentLiveEntity;
}

void Inspector::DrawComponentDisplay(World& currentWorld, ComponentId componentId, void* componentPtr) const
{
    ComponentRegistry::GetComponentInfoById(componentId)
        .DisplayComponent(componentPtr, [&]() { currentWorld.RemoveFromEntity(m_currentLiveEntity, componentId, componentPtr); }, nullptr);
}

void Inspector::DrawInspectorGuiForLiveEntity(EntityManager& entityManager, MoldManager& entityTemplateManager, World& currentWorld)
{
    if (!entityManager.ValidateEntity(m_currentLiveEntity))
    {
        m_inspectorMode = InspectorMode::None;
        m_currentLiveEntity = {};
        return;
    }

    const EntityStorageLocation& entityLocation = entityManager.GetEntityLocation(m_currentLiveEntity);
    Archetype* archetypePtr = entityManager.GetEntityArchetypePtr(m_currentLiveEntity);

    ImGui::SeparatorText("Entity");
    ImGui::Text("Id:%u | Gen:%u", m_currentLiveEntity.id, m_currentLiveEntity.generation);

    ImGui::SeparatorText("Archetype");
    ImGui::Text("Id:%u | Chunk:%u | Index:%u", entityLocation.archetypeId, entityLocation.chunkIndex, entityLocation.indexInChunk);

    ImGui::Spacing();
    ImGui::Spacing();

    archetypePtr->ForEachComponent(entityLocation, [&](ComponentId id, void* ptr) { DrawComponentDisplay(currentWorld, id, ptr); });

    ImGui::SeparatorText("");

    if (ImGui::BeginTable("EntityTemplateTable", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableNextField("Entity Template");
        static std::string name = "";
        InputTextWithHint("##EntityTemplateName", "Enter Name", name);

        ImGui::SameLine();
        if (ImGui::Button("Create"))
        {
            if (name.empty())
            {
                name = "Entity_" + std::to_string(m_currentLiveEntity.id) + "_" + std::to_string(m_currentLiveEntity.generation);
            }
            entityTemplateManager.CreateMold(currentWorld, m_currentLiveEntity, entityLocation, name);
            name = "";
        }
        ImGui::EndTable();
    }

    ImGui::SeparatorText("");

    ImVec2 addComponentBtnSize = ImGui::CalcTextSize("Add Component");
    ImVec2 deleteBtnSize = ImGui::CalcTextSize("Delete");
    ImVec2 availableSpace = ImGui::GetContentRegionAvail();
    float xPos = (availableSpace.x - (addComponentBtnSize.x + deleteBtnSize.x)) * 0.5f;
    ImGui::SetCursorPosX(xPos);

    if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponentPopup");
    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        ImGui::SeparatorText("Components");
        ImGui::BeginChild("ComponentList", ImVec2(125, 150));
        for (auto& componentInfo : ComponentRegistry::GetComponentInfos())
        {
            if (ImGui::Selectable(componentInfo.name))
            {
                currentWorld.AddToEntity(m_currentLiveEntity, componentInfo.id, componentInfo.DefaultConstructComponent());
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::RustRed_SFML);
    if (ImGui::Button("Delete"))
    {
        currentWorld.DestroyEntity(m_currentLiveEntity);

        CMolded* moldedComp = currentWorld.TryGetComponent<CMolded>(m_currentLiveEntity);
        if (moldedComp)
        {
            Mold* entityTemplate = entityTemplateManager.GetMoldByName(moldedComp->moldName);
            assert(entityTemplate);

            std::vector<Entity>& derivedEntites = entityTemplate->derivedEntities;
            size_t index = SIZE_MAX;
            for (size_t i = 0; i < derivedEntites.size(); ++i)
            {
                if (derivedEntites[i].id == m_currentLiveEntity.id)
                {
                    index = i;
                    break;
                }
            }
            if (index != SIZE_MAX)
            {
                derivedEntites[index] = derivedEntites[derivedEntites.size() - 1];
                derivedEntites.pop_back();
            }
            else
            {
                LOG_ERROR("Entity was not found in under it's template.")
            }
        }
    };
    ImGui::PopStyleColor();
}

void Inspector::DrawInspectorGuiForMoldedInstance(MoldManager& entityTemplateManager, World& currentWorld)
{
    if (!m_currentMoldInstance) return;
    ImGui::SeparatorText("Entity Template");

    ImGui::Text("Name: %s", m_currentMoldInstance->GetName().c_str());

    ImGui::Spacing();
    ImGui::Spacing();

    m_currentMoldInstance->DrawInspector();

    ImGui::SeparatorText("");

    if (ImGui::BeginTable("EntityTemplateNameTable", 2, ImGuiTableFlags_SizingFixedFit))
    {
        TableNextField("Change Name");

        static std::string newName = "";
        InputTextWithHint("##EntityTemplateName", "Enter Name", newName);

        ImGui::SameLine();
        if (ImGui::Button("Change"))
        {
            if (!newName.empty())
            {
                m_currentMoldInstance->Rename(entityTemplateManager, newName,currentWorld);
                newName.clear();
            }
        }
        ImGui::EndTable();
    }

    ImGui::SeparatorText("");

    ImVec2 saveBtnSize = ImGui::CalcTextSize("Save");
    ImVec2 deleteBtnSize = ImGui::CalcTextSize("Delete");
    ImVec2 addCompBtnSize = ImGui::CalcTextSize("Add Component");
    ImVec2 space = ImGui::GetContentRegionAvail();
    float xPos = (space.x - (saveBtnSize.x + deleteBtnSize.x + addCompBtnSize.x)) * 0.5f;
    ImGui::SetCursorPosX(xPos);

    if (ImGui::Button("Add Component")) ImGui::OpenPopup("AddComponentPopup");
    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        ImGui::SeparatorText("Components");
        ImGui::BeginChild("ComponentList", ImVec2(125, 150));
        for (const auto& componentInfo : ComponentRegistry::GetComponentInfos())
        {
            if (ImGui::Selectable(componentInfo.name))
            {
                m_currentMoldInstance->AddComponent(&componentInfo, componentInfo.DefaultConstructComponent());
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    bool dirtyFlagTemp = false;
    if (m_currentMoldInstance->IsDirty())
    {
        ImGui::PushStyleColor(ImGuiCol_Border, Colors::OxidizedGreen_ImGui);
        dirtyFlagTemp = true;
    }
    if (ImGui::Button("Save"))
    {
        if (m_lastEngineMode == EngineMode::Edit)
        {
            m_currentMoldInstance->Save(entityTemplateManager, currentWorld);
        }
        else
        {
            LOG_WARNING("Entity Templates can only be modified outside playmode.");
        }
    }
    if (dirtyFlagTemp)
    {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Border, Colors::RustRed_SFML);
    if (ImGui::Button("Delete"))
    {
        entityTemplateManager.DeleteMold(m_currentMoldInstance->GetName());

        m_currentMoldInstance.reset();
        m_inspectorMode = InspectorMode::None;
    }
    ImGui::PopStyleColor();
}

void Inspector::InspectLiveEntity(Entity entity, EntityManager& entityManager)
{
    if (m_currentLiveEntity != entity && entityManager.ValidateEntity(entity))
    {
        m_currentLiveEntity = entity;
        m_inspectorMode = InspectorMode::LiveEntity;
        m_currentMoldInstance.reset();
    }
}

void Inspector::InspectMold(Mold& entityTemplate)
{
    m_currentMoldInstance = std::make_unique<MoldInstance>(entityTemplate);
    m_inspectorMode = InspectorMode::Mold;
    m_currentLiveEntity = {};
}

void Inspector::DrawInspectorGui(EntityManager& entityManager, MoldManager& entityTemplateManager, World& currentWorld,
                                       EngineMode engineMode)
{
    if (m_lastEngineMode != engineMode)
    {
        m_lastEngineMode = engineMode;
        m_currentLiveEntity = {};
    }

    switch (m_inspectorMode)
    {
        case InspectorMode::LiveEntity:
            DrawInspectorGuiForLiveEntity(entityManager, entityTemplateManager, currentWorld);
            break;
        case InspectorMode::Mold:
            DrawInspectorGuiForMoldedInstance(entityTemplateManager, currentWorld);
            break;
        case InspectorMode::None:
            break;
    }
};
