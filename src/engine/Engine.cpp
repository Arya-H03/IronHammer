#include "Engine.h"
#include <imgui-SFML.h>
#include "assets/FontManager.h"
#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include <SFML/Graphics/Color.hpp>
#include <cassert>
#include "Tracy.hpp"

Engine::Engine()
    : m_entityManager(m_archetypeRegistry)
    , m_commandBuffer(m_entityManager)
    , m_renderSystem(m_window, m_archetypeRegistry)
    , m_movementSystem(m_archetypeRegistry)
    , m_collisionSystem(m_entityManager, m_archetypeRegistry, m_commandBuffer, m_windowSize)
    , m_guiSystem(m_entityManager, m_commandBuffer, m_renderSystem, m_archetypeRegistry, m_collisionSystem.GetCollsionDebugger(), m_windowSize)
{
    Init();
}

void Engine::Init()
{
    Random::Init();
    FontManager::InitializeFont();

    m_window.create(sf::VideoMode({ m_windowSize.x, m_windowSize.y }), "IronHammer");
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
        Vect2f startPos = Vect2f(Random::Float(50, m_windowSize.x - 50), Random::Float(50, m_windowSize.y - 50));
        Vect2f startVel = Vect2f(Random::Float(-10, 10), Random::Float(-10, 10));
        float speed = Random::Float(1, 5);

        float shapeRadius = Random::Float(1, 5);
        int points = Random::Int(3, 20);
        sf::Color filColor = Random::Color();

        Entity e;
        m_commandBuffer.CreateEntity(e,
            CTransform(startPos, Vect2f(3, 3), 0),
            CMovement(startVel, speed),
            CCollider(Vect2f(shapeRadius * 2, shapeRadius * 2), Vect2f(0, 0), false),
            CShape(points, filColor, sf::Color::White, shapeRadius, 0));
    }
}

void Engine::Run()
{
    ZoneScoped;

    SpawnTestEntity();

    while (m_window.isOpen())
    {
        FrameMark;

        m_commandBuffer.ExecuteAllCommands();

        ImGui::SFML::Update(m_window, m_clock.restart());
        while (const auto event = m_window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(m_window, *event);
            if (event->is<sf::Event::Closed>()) m_window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                isPaused = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::U))
            {
                isPaused = false;
            }
        }

        if (!isPaused)
        {
            m_movementSystem.HandleMovementSystem();
            m_collisionSystem.HandleCollisionSystem();
        }

        m_guiSystem.HandleGUISystem();

        ///////////Always call LAST/////////
        m_renderSystem.HandleRenderSystem();
        ////////////////////////////////////

        ++m_currentFrame;
    }
}
