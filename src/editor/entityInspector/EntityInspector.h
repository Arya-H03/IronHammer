#pragma once

#include <imgui.h>
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityManager.hpp"
#include "ecs/entity/EntityTemplateManager.h"
#include "ecs/entity/EntityTemplateInstance.h"
#include "engine/Engine.h"

using namespace EntityInspectorHelpers;

class EntityInspector
{

    enum class InspectorMode
    {
        None,
        LiveEntity,
        EntityTemplate
    };

  private:

    Entity m_currentLiveEntity;
    std::unique_ptr<EntityTemplateInstance> m_currentEntityTemplateInstance;
    InspectorMode m_inspectorMode = InspectorMode::None;
    EngineMode m_lastEngineMode = EngineMode::None;

    void DrawComponentDisplay(World& currentWorld,ComponentId componentId, void* componentPtr) const;
    void DrawInspectorGuiForLiveEntity(EntityManager& entityManager, EntityTemplateManager& entityTemplateManager, World& currentWorld);
    void DrawInspectorGuiForEntityTemplate(EntityTemplateManager& entityTemplateManager);

  public:

    EntityInspector() = default;

    const Entity GetCurrentInspectorEntity() const;

    void InspectLiveEntity(Entity entity, EntityManager& entityManager);
    void InspectEntityTemplate(EntityTemplate& entityTemplate);
    void DrawInspectorGui(EntityManager& entityManager, EntityTemplateManager& entityTemplateManager, World& currentWorld, EngineMode engineMode);
};
