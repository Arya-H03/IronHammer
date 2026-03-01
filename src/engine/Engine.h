#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <cstdint>
#include <memory>
#include "ecs/World.hpp"
#include "input/InputSystem.h"
#include "rendering/RenderSystem.h"
#include "core/utils/Vect2.hpp"
#include "scene/SceneManager.h"

enum class EngineMode
{
    Edit,
    Play
};

class Engine
{


  private:

    int m_currentFrame = 0;
    int m_frameLimit = 60;

    sf::Clock m_clock;
    sf::RenderWindow m_window;
    const Vect2<uint16_t> m_windowSize { 1920, 1080 };

    EngineMode m_engineMode = EngineMode::Edit;

    std::unique_ptr<World> m_editorWorld;
    std::unique_ptr<World> m_tempWorld;
    World* m_currentWorld;

    RenderSystem m_renderSystem;
    InputSystem m_inputSystem;
    SceneManager m_sceneManager;

    void EnterPlayMode();
    void ExitPlayMode();

    void Init();

  public:

    Engine();

    bool GetIsPlayModePaused() const;
    EngineMode GetEngineMode()const;
    Vect2<uint16_t> GetWindowSize() const;
    World* GetCurrentWorld() const;

    sf::RenderWindow& GetRenderWindow();
    RenderSystem& GetRenderSystem();
    SceneManager& GetSceneManager();
    InputSystem& GetInputSystem();

    void TogglePlayMode();
    void PausePlayMode();

    float BeginFrame();
    void UpdateRuntime();
    void RenderFrame();
    void EndFrame();
};
