#pragma once
#include <imgui.h>
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "ecs/system/BaseSystem.h"

class GUISystem: public BaseSystem
{
  private:
    EntityManager& m_entityManager;
    ArchetypeDebugger m_archetypeDebugger;

  public:
    GUISystem(EntityManager& entityManager, ArchetypeRegistry& archetypeRegistry)
        : m_entityManager(entityManager), m_archetypeDebugger(entityManager, archetypeRegistry)
    {
    }
    void HandleGUISystem();
    void AppleGUITheme();
};
