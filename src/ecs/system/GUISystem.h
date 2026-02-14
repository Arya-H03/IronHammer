#pragma once
#include <imgui.h>
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/entity/EntityManager.hpp"

class GUISystem
{
  private:
    EntityManager& m_entityManager;
    ArchetypeDebugger m_archetypeDebugger;

  public:
    GUISystem(EntityManager& entityManager) : m_entityManager(entityManager) {}
    void HandleGUISystem();
    void AppleGUITheme();
};
