#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "input/InputSystem.h"
#include "scene/BaseScene.h"
#include "core/utils/Vect2.hpp"

class Engine
{
  private:

    int m_currentFrame = 0;
    int m_frameLimit = 60;

    const Vect2<uint16_t> m_windowSize { 1920, 1080 };

    sf::RenderWindow m_window;
    sf::Clock m_clock;

    InputSystem m_inputSystem;

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
