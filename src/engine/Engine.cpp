#include "Engine.h"
#include <SFML/Window/Keyboard.hpp>
#include <imgui-SFML.h>
#include "assets/FontManager.h"
#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include <SFML/Graphics/Color.hpp>
#include <cassert>
#include "Tracy.hpp"
#include "input/InputManager.h"
#include "core/utils/Debug.h"

Engine::Engine()
    : m_entityManager(m_archetypeRegistry)
    , m_commandBuffer(m_entityManager)
    , m_renderSystem(m_window, m_archetypeRegistry)
    , m_movementSystem(m_archetypeRegistry)
    , m_collisionSystem(m_entityManager, m_archetypeRegistry, m_commandBuffer, m_windowSize)
    , m_inputSystem(m_window)
    , m_inputManager(m_inputSystem)
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
    m_window.setKeyRepeatEnabled(false);

    const bool isWindowInitialized = ImGui::SFML::Init(m_window);
    assert(isWindowInitialized && "Window wasn't initialized");

    m_guiSystem.AppleGUITheme();

    m_inputManager.CreateInputAction("Log", sf::Keyboard::Key::L, InputTrigger::Pressed, [](){Debug::Log("Log");});
    m_inputManager.CreateInputAction("Warning", sf::Keyboard::Key::W, InputTrigger::Pressed, [](){Debug::Warning("Warning");});
    m_inputManager.CreateInputAction("Error", sf::Keyboard::Key::E, InputTrigger::Pressed, [](){Debug::Error("Error");});

}

void Engine::SpawnTestEntity()
{
    ZoneScoped;

    size_t count = 1000;

    for (size_t i = 0; i < count; ++i)
    {
        Vect2f startPos = Vect2f(Random::Float(50, m_windowSize.x - 50), Random::Float(50, m_windowSize.y - 50));
        Vect2f startVel = Vect2f(Random::Float(-90, 90), Random::Float(-90, 90));
        float speed = Random::Float(1, 5);

        float shapeRadius = Random::Float(1, 5);
        int points = Random::Int(3, 20);
        sf::Color filColor = Random::Color();

        Entity e;
        m_commandBuffer.CreateEntity(e,
            CTransform(startPos, Vect2f(3, 3), 45),
            CMovement(speed),
            CRigidBody(startVel, shapeRadius, 0.1f, false),
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
        m_inputSystem.PollEvents();
        m_inputManager.Update();

        m_collisionSystem.HandleCollisionSystem();
        m_movementSystem.HandleMovementSystem();

        m_guiSystem.HandleGUISystem();

        ///////////Always call LAST/////////
        m_renderSystem.HandleRenderSystem();
        ////////////////////////////////////

        m_inputSystem.ClearEvents();
        ++m_currentFrame;
    }
}
