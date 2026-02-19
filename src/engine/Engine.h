#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>
#include <sys/types.h>
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "physics/CollisionSystem.h"
#include "ecs/system/GUISystem.h"
#include "ecs/system/MovementSystem.h"
#include "ecs/system/RenderSystem.h"

class Engine
{
  private:

    int m_currentFrame = 0;
    int m_frameLimit = 0;
    const Vect2<uint16_t> m_windowSize { 1920, 1080 };
    bool isPaused = false;

    sf::RenderWindow m_window;
    sf::Clock m_clock;

    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;

    MovementSystem m_movementSystem;
    CollisionSystem m_collisionSystem;
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
