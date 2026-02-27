#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include "ecs/World.hpp"
#include "input/InputManager.h"
#include "core/utils/Vect2.hpp"

class BaseScene
{
  protected:

    Vect2<uint16_t> m_windowSize;

    World* m_worldPtr;

    InputManager m_inputManager;

  public:

    BaseScene(World* world, InputSystem& inputSystem, Vect2<uint16_t> windowSize)
        : m_worldPtr(world), m_windowSize(windowSize), m_inputManager(inputSystem)
    {
    }

    virtual void OnEnter() { }
    virtual void OnExit() { }

    virtual void Update() = 0;

    virtual ~BaseScene() = default;
};
