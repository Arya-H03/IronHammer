#include "EntityInspector.h"
#include <string>
#include "core/utils/Colors.h"
#include "ecs/entity/EntityInspectorHelper.h"
#include "engine/Engine.h"

const Entity EntityInspector::GetCurrentInspectorEntity() const { return m_currentLiveEntityData.entity; }

void EntityInspector::DrawComponentDisplay(ComponentID componentId, void* componentPtr) const
{
    ComponentRegistry::GetComponentInfoById(componentId).DisplayComponent(componentPtr, nullptr);
}

void EntityInspector::DrawInspectorGuiForLiveEntity(EntityManager& entityManager, EntityTemplateManager& entityTemplateManager, World& currentWorld)
{
    if (m_currentLiveEntityData.entity.id != InvalidEntityID)
    {
        if (!entityManager.ValidateEntity(m_currentLiveEntityData.entity))
        {
            m_inspectorMode = InspectorMode::None;
            m_currentLiveEntityData.Clear();
            return;
        }

        ImGui::SeparatorText("Entity");
        ImGui::Text("Id:%u | Gen:%u", m_currentLiveEntityData.entity.id, m_currentLiveEntityData.entity.generation);
        ImGui::SeparatorText("Archetype");
        ImGui::Text("Id:%u | Chunk:%u | Index:%u",
            m_currentLiveEntityData.location.archetypeId,
            m_currentLiveEntityData.location.chunkIndex,
            m_currentLiveEntityData.location.indexInChunk);

        m_currentLiveEntityData.archetypePtr->ForEachComponent(
            m_currentLiveEntityData.location, [&](ComponentID id, void* ptr) { DrawComponentDisplay(id, ptr); });

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
                    name = "Entity_" + std::to_string(m_currentLiveEntityData.entity.id) + "_"
                           + std::to_string(m_currentLiveEntityData.entity.generation);
                }
                entityTemplateManager.CreateEntityTemplate(currentWorld, m_currentLiveEntityData.entity, m_currentLiveEntityData.location, name);
                name = "";
            }

            ImGui::EndTable();
        }
    }
}

void EntityInspector::DrawInspectorGuiForEntityTemplate(EntityTemplateManager& entityTemplateManager)
{
    if (!m_currentEntityTemplateInstance) return;

    ImGui::SeparatorText("Entity Template");

    ImGui::Text("Name: %s", m_currentEntityTemplateInstance->GetName().c_str());

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
    if (m_currentLiveEntityData.entity != entity && entityManager.ValidateEntity(entity))
    {
        m_currentLiveEntityData.entity = entity;
        m_currentLiveEntityData.location = entityManager.GetEntityLocation(m_currentLiveEntityData.entity);
        m_currentLiveEntityData.archetypePtr = entityManager.GetEntityArchetypePtr(m_currentLiveEntityData.entity);
        m_inspectorMode = InspectorMode::LiveEntity;
        m_currentEntityTemplateInstance.reset();
    }
}

void EntityInspector::InspectEntityTemplate(EntityTemplate& entityTemplate)
{
    m_currentEntityTemplateInstance = std::make_unique<EntityTemplateInstance>(entityTemplate);
    m_inspectorMode = InspectorMode::EntityTemplate;
    m_currentLiveEntityData.Clear();
}

void EntityInspector::DrawInspectorGui(
    EntityManager& entityManager, EntityTemplateManager& entityTemplateManager, World& currentWorld, EngineMode engineMode)
{
    if (m_lastEngineMode != engineMode)
    {
        m_lastEngineMode = engineMode;
        m_currentLiveEntityData.Clear();
    }

    switch (m_inspectorMode)
    {
        case InspectorMode::LiveEntity: DrawInspectorGuiForLiveEntity(entityManager, entityTemplateManager, currentWorld); break;
        case InspectorMode::EntityTemplate: DrawInspectorGuiForEntityTemplate(entityTemplateManager); break;
        case InspectorMode::None: break;
    }
};
