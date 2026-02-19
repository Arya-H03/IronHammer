#pragma once
#include <imgui.h>
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "ecs/system/RenderSystem.h"
#include "physics/CollisionDebugger.h"

class GUISystem
{
  private:
    EntityManager& m_entityManager;
    ArchetypeRegistry& m_archetypeRegistry;
    ArchetypeDebugger m_archetypeDebugger;
    const CollisionDebugger& m_collisionDebugger;
    RenderSystem& m_renderSystem;

  public:
    GUISystem(EntityManager& entityManager,
              RenderSystem& renderSystem,
              ArchetypeRegistry& archetypeRegistry,
              const CollisionDebugger& collisionDebugger)
        : m_entityManager(entityManager),
          m_renderSystem(renderSystem),
          m_archetypeRegistry(archetypeRegistry),
          m_archetypeDebugger(entityManager, archetypeRegistry),
          m_collisionDebugger(collisionDebugger)
    {
    }
    void HandleGUISystem();
    void AppleGUITheme();
};
