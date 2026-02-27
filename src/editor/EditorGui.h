#pragma once
#include <cstdint>
#include <imgui.h>
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/common/ECSCommon.h"
#include "gui/LogWindow.hpp"
#include "rendering/RenderSystem.h"
#include "physics/CollisionDebugger.h"

class EditorGui
{
  private:

    RenderSystem& m_renderSystem;
     sf::RenderTexture& m_renderTexture;
    Vect2<uint16_t> m_windowSize;

    World* m_worldPtr;
    LogWindow m_logWindow;

    const EntityInspector& m_entityInspector;
    const ArchetypeDebugger& m_archetypeDebugger;
    //const CollisionDebugger& m_collisionDebugger;

    const uint16_t m_offsetFromScreenEdge = 2;
    const uint16_t m_inspectorWindowWidth = 350;
    const uint16_t m_inspectorWindowHeight = 750;
    const uint16_t m_debugWindowWidth = 450;
    const uint16_t m_debugWindowHeight = 400;
    const uint16_t m_logWindowWidth = 700;
    const uint16_t m_logWindowHeight = 300;

    void DrawMenuBar();
    void DrawDebugWindow();
    void DrawInspectorWindow();
    void DrawLogWindow();
    void DrawViewport();

  public:

    EditorGui(World* world, RenderSystem& renderSystem, sf::RenderTexture& renderTexture, Vect2<uint16_t> windowSize);
    //GuiSystem(World& world, RenderSystem& renderSystem, const CollisionDebugger& collisionDebugger, Vect2<uint16_t> windowSize);

    void SetCurrentInspectorEntity(Entity entity);
    void HandleGUISystem();
    void ApplyGuiTheme();
};
