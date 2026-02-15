#include "Engine.h"
#include <cassert>

Engine::Engine()
    : m_archetypeRegistry(m_systemRegistry),
      m_entityManager(m_archetypeRegistry),
      m_guiSystem(m_systemRegistry.RegisterSystem<GUISystem>(m_entityManager, m_archetypeRegistry)),
      m_renderSystem(m_systemRegistry.RegisterSystem<RenderSystem>(m_window))
{
    Init();
}

void Engine::Init()
{
   	m_window.create(sf::VideoMode({ 1280, 720}), "IronHammer");
	m_window.setFramerateLimit(m_frameLimit);

    const bool isWindowInitialized = ImGui::SFML::Init(m_window);
    assert(isWindowInitialized && "Window wasn't initialized");

    m_guiSystem.AppleGUITheme();
}

void Engine::Run()
{
    Entity e0 = m_entityManager.CreateEntity(CPosition{1, 1, 1}, CShape(30, 3, sf::Color::Black, sf::Color::Yellow, 2));
    Entity e5 = m_entityManager.CreateEntity(CPosition{1, 1, 1}, CShape(50, 3, sf::Color::Black, sf::Color::Green, 2));
    Entity e6 = m_entityManager.CreateEntity(CPosition{1, 1, 1}, CShape(20, 3, sf::Color::Black, sf::Color::Blue, 2));
    Entity e1 = m_entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e2 = m_entityManager.CreateEntity(CPosition{1, 1, 1}, CVelocity(0, 100, 12));
    Entity e3 = m_entityManager.CreateEntity(CRotation(2, 2, 1));
    Entity e4 = m_entityManager.CreateEntity(CVelocity(0, 100, 12), CRotation(2, 2, 2));

    // entityManager.GetComponentRef<CShape>(e0).shape.setPosition({50, 50});
    // entityManager.GetComponentRef<CShape>(e5).shape.setPosition({100, 100});
    // entityManager.GetComponentRef<CShape>(e6).shape.setPosition({200, 200});

    while (m_window.isOpen())
    {
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

        m_guiSystem.HandleGUISystem();
        m_renderSystem.HandleRenderSystem();

        ++m_currentFrame;
    }
}
