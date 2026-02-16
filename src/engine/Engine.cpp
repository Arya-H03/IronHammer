#include "Engine.h"
#include "core/utils/Random.hpp"
#include "ecs/component/Components.hpp"
#include "ecs/system/MovementSystem.h"
#include <SFML/System/Vector2.hpp>
#include <cassert>

Engine::Engine()
    : m_archetypeRegistry(m_systemRegistry),
      m_entityManager(m_archetypeRegistry),
      m_guiSystem(m_systemRegistry.RegisterSystem<GUISystem>(m_entityManager, m_archetypeRegistry)),
      m_renderSystem(m_systemRegistry.RegisterSystem<RenderSystem>(m_window)),
      m_movementSystem(m_systemRegistry.RegisterSystem<MovementSystem>())
{
    Init();
}

void Engine::Init()
{
    Random::Init();

    m_window.create(sf::VideoMode({1280, 720}), "IronHammer");
    m_window.setFramerateLimit(m_frameLimit);

    const bool isWindowInitialized = ImGui::SFML::Init(m_window);
    assert(isWindowInitialized && "Window wasn't initialized");

    m_guiSystem.AppleGUITheme();
}

void Engine::Run()
{
    //   Entity e0 = m_entityManager.CreateEntity(CShape(30, 3, sf::Color::Black, sf::Color::Yellow, 2));
    // Entity e5 = m_entityManager.CreateEntity(CShape(50, 3, sf::Color::Black, sf::Color::Green, 2));
    // Entity e6 = m_entityManager.CreateEntity(CShape(20, 3, sf::Color::Black, sf::Color::Blue, 2));
    Entity e1 = m_entityManager.CreateEntity(CTransform(Vect2f(100, 100), Vect2f(2, 2), Vect2f(3, 3)),
                                             CMovement(Vect2f(5, 5), 5),
                                             CShape(50, 3, sf::Color::Black, sf::Color::Green, 2));
    //    Entity e2 = m_entityManager.CreateEntity(CTransform(Vect2f(1, 1), Vect2f(2, 2), Vect2f(3, 3)), CMovement(Vect2f(7,
    //    7)));

    while (m_window.isOpen())
    {
        std::cerr<<Random::Float(0, 10)<<"\n";
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
        m_guiSystem.HandleGUISystem();
        m_renderSystem.HandleRenderSystem();

        ++m_currentFrame;
    }
}
