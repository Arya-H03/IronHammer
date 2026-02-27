#pragma once
#include <cstdint>
#include <imgui.h>
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/common/ECSCommon.h"
#include "editor/EditorConfig.h"
#include "gui/LogWindow.hpp"
#include "rendering/RenderSystem.h"
#include "physics/CollisionDebugger.h"

class EditorGui
{
  private:

    RenderSystem& m_renderSystem;
    sf::RenderTexture& m_renderTexture;
    EditorConfig::Layout& m_editorLayout;

    World* m_worldPtr;
    LogWindow m_logWindow;

    const EntityInspector& m_entityInspector;
    const ArchetypeDebugger& m_archetypeDebugger;
    // const CollisionDebugger& m_collisionDebugger;

    const uint16_t m_menuBarHeight = 25;
    const uint16_t m_offsetFromScreenEdge = 2;
    const uint16_t m_inspectorWindowWidth = 350;
    const uint16_t m_debugWindowWidth = 450;
    const uint16_t m_logWindowHeight = 200;

    void DrawMenuBar();
    void DrawDebugWindow();
    void DrawInspectorWindow();
    void DrawLogWindow();
    void DrawViewport();

  public:

    EditorGui(
        World* world, EditorConfig::Layout& editorLayout, RenderSystem& renderSystem, sf::RenderTexture& renderTexture, Vect2<uint16_t> windowSize);
    // GuiSystem(World& world, RenderSystem& renderSystem, const CollisionDebugger& collisionDebugger, Vect2<uint16_t> windowSize);

    void SetCurrentInspectorEntity(Entity entity);
    void HandleGUISystem();
    void ApplyGuiTheme();
};
