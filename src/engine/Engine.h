#pragma once

#include "core/FrameRateHandler.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "mold/MoldManager.h"
#include "input/InputSystem.h"
#include "rendering/RenderSystem.h"
#include "scene/SceneManager.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <cstdint>
#include <memory>

enum class EngineMode
{
    None,
    Edit,
    Play,
    Both
};

class Engine
{

  private:
    size_t m_currentFrame = 0;

    sf::Clock m_clock;
    sf::RenderWindow m_window;
    const Vect2<uint16_t> m_windowSize{1920, 1080};

    EngineMode m_engineMode = EngineMode::Edit;

    std::unique_ptr<World> m_editorWorld;
    std::unique_ptr<World> m_tempWorld;
    World* m_currentWorld;

    FrameRateHandler m_frameRateHandler;
    RenderingSystem m_renderSystem;
    InputSystem m_inputSystem;
    SceneManager m_sceneManager;
    MoldManager m_moldManager;

    void EnterPlayMode();
    void ExitPlayMode();

    void Init();

  public:
    Engine();

    bool GetIsPlayModePaused() const;
    EngineMode GetEngineMode() const;
    Vect2<uint16_t> GetWindowSize() const;
    World* GetCurrentWorld() const;
    size_t GetFrameLimit() const;
    float GetTargetFrameTime() const;

    sf::RenderWindow& GetRenderWindow();
    RenderingSystem& GetRenderSystem();
    SceneManager& GetSceneManager();
    InputSystem& GetInputSystem();
    MoldManager& GetMoldTemplateManager();

    void TogglePlayMode();
    void PausePlayMode();

    void BeginFrame();
    void UpdateRuntime();
    void RenderFrame();
    void EndFrame();

    void SaveEditWorldData();
    void LoadEditorSceneData();
    void LoadTempSceneData();
};
