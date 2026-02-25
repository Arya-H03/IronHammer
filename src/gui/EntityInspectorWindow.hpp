#pragma once
#include <imgui.h>
#include "ecs/archetype/Archetype.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"

class EntityInspectorWindow
{
    struct InspectorEntityData
    {
        Entity entity {};
        Archetype* archetype = nullptr;
        EntityStorageLocation location {};
    };

  private:

    InspectorEntityData m_inspectorEntityData;
    EntityManager& m_entityManager;

    void DrawComponentDisplay(ComponentID componentId, void* componentPtr) const
    {
        ComponentRegistry::GetComponentInfoById(componentId).DisplayComponent(componentPtr);
    }

  public:

    EntityInspectorWindow(EntityManager& entityManager) : m_entityManager(entityManager){ }

    void SetCurrentInspectorEntity(Entity entity)
    {
        if (m_inspectorEntityData.entity != entity)
        {
            m_inspectorEntityData.entity = entity;
            m_inspectorEntityData.location = m_entityManager.m_entityStorageLocations[m_inspectorEntityData.entity.id];
            m_inspectorEntityData.archetype = &m_entityManager.m_archetypeRegistry.GetArchetypeById(m_inspectorEntityData.location.archetypeId);
        }
    }

    const Entity GetCurrentInspectorEntity() const { return m_inspectorEntityData.entity; }

    void DrawInspectorGui()
    {
        if (m_inspectorEntityData.entity.id != InvalidEntityID && m_entityManager.ValidateEntity(m_inspectorEntityData.entity))
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
        }
    };
};
