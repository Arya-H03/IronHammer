#pragma once
#include <imgui.h>
#include <string>
#include "core/utils/Debug.h"
#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"
#include "ecs/entity/EntityManager.hpp"

using namespace EntityInspectorHelpers;

class EntityInspector
{
    struct InspectorEntityData
    {
        Entity entity {};
        Archetype* archetype = nullptr;
        EntityStorageLocation location {};
    };

  private:

    InspectorEntityData m_inspectorEntityData;
    void DrawComponentDisplay(ComponentID componentId, void* componentPtr) const
    {
        ComponentRegistry::GetComponentInfoById(componentId).DisplayComponent(componentPtr);
    }

  public:

    EntityInspector() = default;

    void SetCurrentInspectorEntity(Entity entity, EntityManager& entityManager)
    {
        if (m_inspectorEntityData.entity != entity)
        {
            m_inspectorEntityData.entity = entity;
            m_inspectorEntityData.location = entityManager.m_entityStorageLocations[m_inspectorEntityData.entity.id];
            m_inspectorEntityData.archetype = &entityManager.m_archetypeRegistry.GetArchetypeById(m_inspectorEntityData.location.archetypeId);
        }
    }

    const Entity GetCurrentInspectorEntity() const { return m_inspectorEntityData.entity; }

    void DrawInspectorGui(EntityManager& entityManager) const
    {
        if (m_inspectorEntityData.entity.id != InvalidEntityID && entityManager.ValidateEntity(m_inspectorEntityData.entity))
        {
            ImGui::SeparatorText("Entity");
            ImGui::Text("Id:%u | Gen:%u", m_inspectorEntityData.entity.id, m_inspectorEntityData.entity.generation);
            ImGui::SeparatorText("Archetype");
            ImGui::Text("Id:%u | Chunk:%u | Index:%u",
                m_inspectorEntityData.location.archetypeId,
                m_inspectorEntityData.location.chunkIndex,
                m_inspectorEntityData.location.indexInChunk);

            m_inspectorEntityData.archetype->ForEachComponent(
                m_inspectorEntityData.location, [&](ComponentID id, void* ptr) { DrawComponentDisplay(id, ptr); });

            ImGui::SeparatorText("");

            if (ImGui::BeginTable("EntityTemplateTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Entity Template");
                static std::string name = "";
                InputTextWithHint("##EntityTemplateName", "Enter Name", name);
                ImGui::SameLine();
                if (ImGui::Button("Create"))
                {
                    if (name == "")
                    {
                        name = "Entity_" + std::to_string(m_inspectorEntityData.entity.id);
                    }
                    Log_Warning("We balling " + name);
                }

                ImGui::EndTable();
            }
        }
    };
};
