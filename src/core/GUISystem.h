#pragma once
#include <imgui.h>
#include "ArchetypeDebugger.hpp"
#include "EntityManager.hpp"

class GUISystem
{
  private:
    EntityManager& m_entityManager;
    ArchetypeDebugger archetypeDEbugger;

  public:
    GUISystem(EntityManager& entityManager) : m_entityManager(entityManager) {}
    void HandleGUISystem();
    void AppleGUITheme();
};
