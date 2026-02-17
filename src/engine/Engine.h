#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <sys/types.h>
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "ecs/system/CollisionSystem.h"
#include "ecs/system/GUISystem.h"
#include "ecs/system/MovementSystem.h"
#include "ecs/system/RenderSystem.h"

class Engine
{
  private:
    sf::RenderWindow m_window;
    sf::Clock m_clock;

    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;

    GUISystem m_guiSystem;
    RenderSystem m_renderSystem;
    MovementSystem m_movementSystem;
    CollisionSystem m_collisionSystem;

    int m_currentFrame = 0;
    int m_frameLimit = 0;
    uint m_windowWidth = 1280;
    uint m_windowHeight = 720;

    void Init();
    void CloseWindow();
    void SpawnTestEntity();

  public:
    Engine();
    void Run();
};
