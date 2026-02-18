#include "Engine.h"
#include "assets/FontManager.h"
#include "core/utils/CustomTypes.hpp"
#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include <SFML/Graphics/Color.hpp>
#include <cassert>
#include "Tracy.hpp"

Engine::Engine()
    : m_entityManager(m_archetypeRegistry),
      m_guiSystem(m_entityManager, m_archetypeRegistry),
      m_renderSystem(m_window, m_archetypeRegistry),
      m_movementSystem(m_archetypeRegistry),
      m_collisionSystem(m_entityManager, m_archetypeRegistry, m_windowSize)
{
    Init();
}

void Engine::Init()
{
    Random::Init();
    FontManager::InitializeFont();

    m_window.create(sf::VideoMode({m_windowSize.x, m_windowSize.y}), "IronHammer");
    m_window.setFramerateLimit(m_frameLimit);

    const bool isWindowInitialized = ImGui::SFML::Init(m_window);
    assert(isWindowInitialized && "Window wasn't initialized");

    m_guiSystem.AppleGUITheme();
}

void Engine::SpawnTestEntity()
{
    ZoneScoped;

    size_t count = 10000;

    for (size_t i = 0; i < count; ++i)
    {
        Vect2f startPos = Vect2f(Random::Float(250, m_windowSize.x - 250), Random::Float(250, m_windowSize.y - 250));
        Vect2f startVel = Vect2f(Random::Float(-10, 10), Random::Float(-10, 10));
        float speed = Random::Float(1, 5);

        float shapeRadius = Random::Float(1, 5);
        int points = Random::Int(3, 32);
        sf::Color filColor = Random::Color();

        m_entityManager.CreateEntity(CTransform(startPos, 0, Vect2f(3, 3)),
                                     CMovement(startVel, speed),
                                     CCollider(BoundingBox(Vect2f(0, 0), Vect2f(shapeRadius, shapeRadius)), false),
                                     CShape(shapeRadius, points, filColor, sf::Color::White, 1));
    }
}

void Engine::Run()
{
    ZoneScoped;

    SpawnTestEntity();

    while (m_window.isOpen())
    {
        FrameMark;
        ImGui::SFML::Update(m_window, m_clock.restart());
        while (const auto event = m_window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(m_window, *event);
            if (event->is<sf::Event::Closed>()) m_window.close();
            // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            // {
            //     entityManager.AddToEntity(e2, CRotation{5,4,3});
            // }
        }

        m_movementSystem.HandleMovementSystem();
        m_collisionSystem.HandleCollisionSystem();
        m_guiSystem.HandleGUISystem();

        ///////////Always call LAST/////////
        m_renderSystem.HandleRenderSystem();
        ////////////////////////////////////

        ++m_currentFrame;
    }
}
