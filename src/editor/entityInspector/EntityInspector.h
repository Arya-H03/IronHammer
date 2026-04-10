#pragma once

#include "ecs/World.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityManager.hpp"
#include "engine/Engine.h"
#include "mold/MoldInstance.h"
#include "mold/MoldManager.h"

#include <imgui.h>

using namespace EntityInspectorHelpers;

class Inspector
{

    enum class InspectorMode
    {
        None,
        LiveEntity,
        Mold
    };

  private:
    std::unique_ptr<MoldInstance> m_currentMoldInstance;

    Entity m_currentLiveEntity;
    InspectorMode m_inspectorMode = InspectorMode::None;
    EngineMode m_lastEngineMode = EngineMode::None;

    void DrawComponentDisplay(World& currentWorld, ComponentId componentId, void* componentPtr) const;
    void DrawInspectorGuiForLiveEntity(EntityManager& entityManager, MoldManager& entityTemplateManager, World& currentWorld);
    void DrawInspectorGuiForMoldedInstance(MoldManager& entityTemplateManager, World& currentWorld, World& editorWorld);

  public:
    Inspector() = default;

    const Entity GetCurrentInspectorEntity() const;

    void InspectLiveEntity(Entity entity, EntityManager& entityManager);
    void InspectMold(Mold& entityTemplate);
    void DrawInspectorGui(EntityManager& entityManager, MoldManager& entityTemplateManager, World& currentWorld, World& editorWorld,
                          EngineMode engineMode);
};
