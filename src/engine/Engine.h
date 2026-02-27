#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "ecs/World.hpp"
#include "editor/Editor.h"
#include "input/InputSystem.h"
#include "rendering/RenderSystem.h"
#include "scene/BaseScene.h"
#include "core/utils/Vect2.hpp"
#include "core/utils/Random.hpp"

class Engine
{
  private:

    int m_currentFrame = 0;
    int m_frameLimit = 60;

    const Vect2<uint16_t> m_windowSize { 1920, 1080 };
    const Vect2<uint16_t> m_gameWindowSize { 1000, 1000 };
    sf::RenderWindow m_window;

    sf::Clock m_clock;

    std::unique_ptr<World> m_editorWorld;
    std::unique_ptr<World> m_tempWorld;
    World* m_currentWorld;

    RenderSystem m_renderSystem;
    InputSystem m_inputSystem;
    Editor m_editor;

    std::unordered_map<std::string, std::unique_ptr<BaseScene>> m_scenes;
    BaseScene* m_currentScene = nullptr;

    void Init();
    void Update();

  public:

    Engine();
    void Run();

    void RegisterScene(const std::string& name, std::unique_ptr<BaseScene> scene);
    void ChangeScene(const std::string& name);

};
