#pragma once
#include <cstdint>
#include <imgui.h>
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/entity/EntityCommands.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "gui/EntityInspectorWindow.hpp"
#include "gui/LogWindow.hpp"
#include "rendering/RenderSystem.h"
#include "physics/CollisionDebugger.h"

class GuiSystem
{
  private:

    EntityManager& m_entityManager;
    ArchetypeRegistry& m_archetypeRegistry;
    CommandBuffer& m_commandBuffer;
    RenderSystem& m_renderSystem;
    Vect2<uint16_t> m_windowSize;

    EntityInspectorWindow m_entityInspector;
    LogWindow m_logWindow;
    const ArchetypeDebugger m_archetypeDebugger;
    const CollisionDebugger& m_collisionDebugger;

    const uint16_t m_offsetFromScreenEdge = 2;
    const uint16_t m_inspectorWindowWidth = 350;
    const uint16_t m_inspectorWindowHeight = 750;
    const uint16_t m_debugWindowWidth = 450;
    const uint16_t m_debugWindowHeight = 400;
    const uint16_t m_logWindowWidth = 700;
    const uint16_t m_logWindowHeight = 300;

    void DrawDebugWindow();
    void DrawInspectorWindow();
    void DrawLogWindow();

  public:

    GuiSystem(EntityManager& entityManager,
        CommandBuffer& commandBuffer,
        RenderSystem& renderSystem,
        ArchetypeRegistry& archetypeRegistry,
        const CollisionDebugger& collisionDebugger,
        Vect2<uint16_t> windowSize);

    void SetCurrentInspectorEntity(Entity entity);
    void HandleGUISystem();
    void AppleGUITheme();
};
