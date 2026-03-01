#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include "ecs/World.hpp"
#include "input/InputManager.h"
#include "core/utils/Vect2.hpp"

class BaseScene
{
  protected:

    bool m_isPaused = false;
    bool m_isPlaying = false;

    Vect2<uint16_t> m_windowSize;
    World* m_worldPtr;

    InputManager m_inputManager;

  public:

    bool GetScenePlaying() const { return m_isPlaying; }
    bool GetScenePaused() const { return m_isPaused; }
    void SetScenePaused(bool val) { m_isPaused = val; }

    BaseScene(Vect2<uint16_t> windowSize) : m_windowSize(windowSize) { }

    virtual void OnStartPlay(World* worldPtr) = 0;
    virtual void OnExitPlay(World* worldPtr) = 0;
    virtual void OnChangeTo(World* worldPtr) { }
    virtual void OnChangeFrom(World* worldPtr) { }
    virtual void Update(World* worldPtr,InputSystem& inputSystem) = 0;

    virtual ~BaseScene() = default;
};
