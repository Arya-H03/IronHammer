#pragma once
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "ecs/system/GUISystem.h"
#include "ecs/system/MovementSystem.h"
#include "ecs/system/RenderSystem.h"
#include "ecs/system/SystemRegistry.hpp"

class Engine
{
  private:
    sf::RenderWindow m_window;
    sf::Clock  m_clock;

    SystemRegistry m_systemRegistry;
    ArchetypeRegistry m_archetypeRegistry;
    EntityManager m_entityManager;

    GUISystem& m_guiSystem;
    RenderSystem& m_renderSystem;
    MovementSystem& m_movementSystem;

    int m_currentFrame = 0;
    int m_frameLimit = 60;

    void Init();
    void CloseWindow();

  public:
    Engine();
    void Run();
};
