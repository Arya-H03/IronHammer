#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include "ecs/World.hpp"
#include "input/InputManager.h"
#include "rendering/RenderSystem.h"
#include "core/utils/Vect2.hpp"

class BaseScene
{
  protected:

    bool m_isPaused = false;
    Vect2<uint16_t> m_windowSize;

    World m_world;

    InputManager m_inputManager;
    RenderSystem m_renderSystem;

  public:

    BaseScene(sf::RenderWindow& window, InputSystem& inputSystem, Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize), m_inputManager(inputSystem), m_renderSystem(m_world, window)
    {
    }

    virtual void OnEnter() { }
    virtual void OnExit() { }

    virtual void Update() = 0;

    virtual ~BaseScene() = default;
};
