#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include "ecs/World.hpp"
#include "editor/Editor.h"
#include "input/InputSystem.h"
#include "rendering/RenderSystem.h"
#include "core/utils/Vect2.hpp"
#include "scene/SceneManager.h"

class Engine
{
  private:

    int m_currentFrame = 0;
    int m_frameLimit = 60;

    const Vect2<uint16_t> m_windowSize { 1920, 1080 };
    sf::RenderWindow m_window;

    sf::Clock m_clock;

    std::unique_ptr<World> m_editorWorld;
    std::unique_ptr<World> m_tempWorld;
    World* m_currentWorld;

    RenderSystem m_renderSystem;
    InputSystem m_inputSystem;
    SceneManager m_sceneManager;
    Editor m_editor;



    void Init();
    void Update();

  public:

    Engine();
    void Run();



};
