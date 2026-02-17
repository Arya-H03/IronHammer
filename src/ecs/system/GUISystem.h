#pragma once
#include <imgui.h>
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"

class GUISystem
{
  private:
    EntityManager& m_entityManager;
    ArchetypeRegistry& m_archetypeRegistry;
    ArchetypeDebugger m_archetypeDebugger;

  public:
    GUISystem(EntityManager& entityManager, ArchetypeRegistry& archetypeRegistry)
        : m_entityManager(entityManager), m_archetypeRegistry(archetypeRegistry),m_archetypeDebugger(entityManager, archetypeRegistry)
    {
    }
    void HandleGUISystem();
    void AppleGUITheme();
};
