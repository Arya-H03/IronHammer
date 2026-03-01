#pragma once
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/System/String.hpp>

#include <imgui.h>
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "ecs/common/ECSCommon.h"
#include "editor/EditorConfig.h"
#include "engine/Engine.h"
#include "gui/LogWindow.hpp"
#include "rendering/RenderSystem.h"
#include "physics/CollisionDebugger.h"
struct EditorGuiCallbacks
{
    std::function<void()> PlayCurrentScene;
    std::function<void()> PauseCurrentScene;
    std::function<void()> ExitEngine;

    EditorGuiCallbacks(const std::function<void()>& play, const std::function<void()>& pause, const std::function<void()>& exit)
        : PlayCurrentScene(play), PauseCurrentScene(pause), ExitEngine(exit)
    {
    }
};

class EditorGui
{
  private:

    EditorConfig::Layout& m_editorLayout;

    LogWindow m_logWindow;

    // const CollisionDebugger& m_collisionDebugger;
    EditorGuiCallbacks& m_editorGuiCallbacks;

    const std::string playButtonPath = "assets/play-button.png";
    sf::Texture playButtonTexture;
    ImTextureID playButtonTexID;

    const std::string pauseButtonPath = "assets/pause-button.png";
    sf::Texture pauseButtonTexture;
    ImTextureID pauseButtonTexID = (ImTextureID) pauseButtonTexture.getNativeHandle();

    const std::string exitButtonPath = "assets/cross-button.png";
    sf::Texture exitButtonTexture;
    ImTextureID exitButtonTexID = (ImTextureID) exitButtonTexture.getNativeHandle();

    void DrawMenuBar(EngineMode engineMode, bool isPlayModePaused);
    void DrawDebugWindow(const ArchetypeDebugger& archetypeDebugger, RenderSystem& renderSystem);
    void DrawInspectorWindow(const EntityInspector& entityInspector);
    void DrawLogWindow();
    void DrawViewport(sf::RenderTexture& renderTexture);

  public:

    EditorGui(EditorConfig::Layout& editorLayout, EditorGuiCallbacks& callBacks);
    // GuiSystem(World& world, RenderSystem& renderSystem, const CollisionDebugger& collisionDebugger, Vect2<uint16_t> windowSize);

    void SetCurrentInspectorEntity(Entity entity);
    void HandleGUISystem(RenderSystem& renderSystem,
        const ArchetypeDebugger& archetypeDebugger,
        const EntityInspector& entityInspector,
        sf::RenderTexture& renderTexture,
        EngineMode engineMode,
        bool isPlayModePaused);
    void ApplyGuiTheme();
};
