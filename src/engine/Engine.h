#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/InputStream.hpp>
#include <cstdint>
#include <sys/types.h>
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityCommands.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "input/InputManager.h"
#include "physics/CollisionSystem.h"
#include "gui/GUISystem.h"
#include "physics/MovementSystem.h"
#include "rendering/RenderSystem.h"
#include "input/InputSystem.h"

class Engine
{
  private:

    int m_currentFrame = 0;
    int m_frameLimit = 60;
    const Vect2<uint16_t> m_windowSize { 1920, 1080 };
    bool isPaused = false;

    sf::RenderWindow m_window;
    sf::Clock m_clock;

    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;
    CommandBuffer m_commandBuffer;

    MovementSystem m_movementSystem;
    CollisionSystem m_collisionSystem;
    InputSystem m_inputSystem;
    InputManager m_inputManager;
    RenderSystem m_renderSystem;
    //////Declare Last////
    GUISystem m_guiSystem;
    /////////////////////
    void Init();

    void CloseWindow();
    void SpawnTestEntity();

  public:

    Engine();
    void Run();
};
