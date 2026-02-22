#pragma once
#include <cstdint>
#include <imgui.h>
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityCommands.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "gui/EntityInspector.hpp"
#include "ecs/system/RenderSystem.h"
#include "physics/CollisionDebugger.h"

class GUISystem
{
  private:

    EntityManager& m_entityManager;
    ArchetypeRegistry& m_archetypeRegistry;
    CommandBuffer& m_commandBuffer;
    RenderSystem& m_renderSystem;
    Vect2<uint16_t> m_windowSize;

    EntityInspector m_entityInspector;
    const ArchetypeDebugger m_archetypeDebugger;
    const CollisionDebugger& m_collisionDebugger;

    void DrawDebugGuiWindow();

  public:

    GUISystem(EntityManager& entityManager,
        CommandBuffer& commandBuffer,
        RenderSystem& renderSystem,
        ArchetypeRegistry& archetypeRegistry,
        const CollisionDebugger& collisionDebugger,
        Vect2<uint16_t> windowSize)
        : m_entityManager(entityManager)
        , m_commandBuffer(commandBuffer)
        , m_renderSystem(renderSystem)
        , m_archetypeRegistry(archetypeRegistry)
        , m_windowSize(windowSize)
        , m_entityInspector(m_entityManager, m_windowSize)
        , m_collisionDebugger(collisionDebugger)
        , m_archetypeDebugger(entityManager, archetypeRegistry, m_commandBuffer, m_entityInspector)
    {
    }

    void SetCurrentInspectorEntity(Entity entity);
    void HandleGUISystem();
    void AppleGUITheme();
};
