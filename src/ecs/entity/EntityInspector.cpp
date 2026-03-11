#include "EntityInspector.h"
#include <string>
#include "core/utils/Colors.h"
#include "ecs/World.hpp"
#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityInspectorHelper.h"
#include "engine/Engine.h"
#include "imgui.h"

const Entity EntityInspector::GetCurrentInspectorEntity() const { return m_currentLiveEntity; }

void EntityInspector::DrawComponentDisplay(World& currentWorld, ComponentID componentId, void* componentPtr) const
{
    ComponentRegistry::GetComponentInfoById(componentId)
        .DisplayComponent(componentPtr, [&]() { currentWorld.RemoveFromEntity(m_currentLiveEntity, componentId, componentPtr); }, nullptr);
}

void EntityInspector::DrawInspectorGuiForLiveEntity(EntityManager& entityManager, EntityTemplateManager& entityTemplateManager, World& currentWorld)
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

    archetypePtr->ForEachComponent(entityLocation, [&](ComponentID id, void* ptr) { DrawComponentDisplay(currentWorld,id, ptr); });

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

    // Calculate total width of both buttons
    const float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
    ImVec2 saveSize = ImGui::CalcTextSize("Save");
    ImVec2 deleteSize = ImGui::CalcTextSize("Delete");
    float saveButtonWidth = saveSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float deleteButtonWidth = deleteSize.x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float totalWidth = saveButtonWidth + buttonSpacing + deleteButtonWidth;
    float columnWidth = ImGui::GetColumnWidth();
    // Center horizontally
    float cursorX = ImGui::GetCursorPosX() + (columnWidth - totalWidth) * 0.5f;
    ImGui::SetCursorPosX(cursorX);

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

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Border, Colors::RustRed_SFML);
    if (ImGui::Button("Delete"))
    {
        entityTemplateManager.DeleteEntityTemplate(m_currentEntityTemplateInstance->GetName());

        m_currentEntityTemplateInstance.reset();
        m_inspectorMode = InspectorMode::None;
    }
    ImGui::PopStyleColor();
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

void EntityInspector::DrawInspectorGui(
    EntityManager& entityManager, EntityTemplateManager& entityTemplateManager, World& currentWorld, EngineMode engineMode)
{
    if (m_lastEngineMode != engineMode)
    {
        m_lastEngineMode = engineMode;
        m_currentLiveEntity = {};
    }

    switch (m_inspectorMode)
    {
        case InspectorMode::LiveEntity: DrawInspectorGuiForLiveEntity(entityManager, entityTemplateManager, currentWorld); break;
        case InspectorMode::EntityTemplate: DrawInspectorGuiForEntityTemplate(entityTemplateManager); break;
        case InspectorMode::None: break;
    }
};
