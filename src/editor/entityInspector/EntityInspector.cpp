#include "EntityInspector.h"

#include "core/utils/Colors.h"
#include "ecs/World.hpp"
#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"
#include "editor/entityInspector/EntityInspectorHelper.h"
#include "engine/Engine.h"
#include "imgui.h"

#include <string>

const Entity EntityInspector::GetCurrentInspectorEntity() const
{
    return m_currentLiveEntity;
}

void EntityInspector::DrawComponentDisplay(World& currentWorld, ComponentId componentId, void* componentPtr) const
{
    ComponentRegistry::GetComponentInfoById(componentId)
        .DisplayComponent(componentPtr, [&]() { currentWorld.RemoveFromEntity(m_currentLiveEntity, componentId, componentPtr); }, nullptr);
}

void EntityInspector::DrawInspectorGuiForLiveEntity(EntityManager& entityManager, EntityTemplateManager& entityTemplateManager,
                                                    World& currentWorld)
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
            entityTemplateManager.CreateEntityTemplate(currentWorld, m_currentLiveEntity, entityLocation, name);
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
    };
    ImGui::PopStyleColor();
}

void EntityInspector::DrawInspectorGuiForEntityTemplate(EntityTemplateManager& entityTemplateManager)
{
    if (!m_currentEntityTemplateInstance) return;

    ImGui::SeparatorText("Entity Template");

    ImGui::Text("Name: %s", m_currentEntityTemplateInstance->GetName().c_str());

    ImGui::Spacing();
    ImGui::Spacing();

    m_currentEntityTemplateInstance->DrawInspector();

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
                m_currentEntityTemplateInstance->Rename(entityTemplateManager, newName);
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
                m_currentEntityTemplateInstance->AddComponent(&componentInfo, componentInfo.DefaultConstructComponent());
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
        entityTemplateManager.DeleteEntityTemplate(m_currentEntityTemplateInstance->GetName());

        m_currentEntityTemplateInstance.reset();
        m_inspectorMode = InspectorMode::None;
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();
    bool dirtyFlagTemp = false;
    if (m_currentEntityTemplateInstance->IsDirty())
    {
        ImGui::PushStyleColor(ImGuiCol_Border, Colors::OxidizedGreen_ImGui);
        dirtyFlagTemp = true;
    }
    if (ImGui::Button("Save"))
    {
        m_currentEntityTemplateInstance->Save(entityTemplateManager);
    }
    if (dirtyFlagTemp)
    {
        ImGui::PopStyleColor();
    }
}

void EntityInspector::InspectLiveEntity(Entity entity, EntityManager& entityManager)
{
    if (m_currentLiveEntity != entity && entityManager.ValidateEntity(entity))
    {
        m_currentLiveEntity = entity;
        m_inspectorMode = InspectorMode::LiveEntity;
        m_currentEntityTemplateInstance.reset();
    }
}

void EntityInspector::InspectEntityTemplate(EntityTemplate& entityTemplate)
{
    m_currentEntityTemplateInstance = std::make_unique<EntityTemplateInstance>(entityTemplate);
    m_inspectorMode = InspectorMode::EntityTemplate;
    m_currentLiveEntity = {};
}

void EntityInspector::DrawInspectorGui(EntityManager& entityManager, EntityTemplateManager& entityTemplateManager, World& currentWorld,
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
        case InspectorMode::EntityTemplate:
            DrawInspectorGuiForEntityTemplate(entityTemplateManager);
            break;
        case InspectorMode::None:
            break;
    }
};
