#include "GameScene.h"
#include "core/utils/Random.hpp"

GameScene::GameScene(sf::RenderWindow& window, InputSystem& inputSystem, Vect2<uint16_t> windowSize)
    : BaseScene(window, inputSystem, windowSize)
    , m_movementSystem(m_world)
    , m_collisionSystem(m_world, m_windowSize)
    , m_guiSystem(m_world, m_renderSystem, m_collisionSystem.GetCollsionDebugger(), m_windowSize)
{
}

void GameScene::OnEnter()
{
    m_guiSystem.AppleGUITheme();

    m_inputManager.CreateInputAction("Log", sf::Keyboard::Key::L, InputTrigger::Pressed, []() { Log_Info("Info") });
    m_inputManager.CreateInputAction("Warning", sf::Keyboard::Key::W, InputTrigger::Pressed, []() { Log_Warning("Warnings"); });
    m_inputManager.CreateInputAction("Error", sf::Keyboard::Key::E, InputTrigger::Pressed, []() { Log_Error("Error"); });

    SpawnTestEntities();
}

void GameScene::OnExit() { }

void GameScene::Update()
{
    m_world.UpdateWorld();

    m_inputManager.Update();
    m_collisionSystem.HandleCollisionSystem();
    m_movementSystem.HandleMovementSystem();
    m_guiSystem.HandleGUISystem();

    m_renderSystem.HandleRenderSystem();
}

void GameScene::SpawnTestEntities()
{
    size_t count = 1000;

    for (size_t i = 0; i < count; ++i)
    {
        Vect2f startPos { Random::Float(50, m_windowSize.x - 50), Random::Float(50, m_windowSize.y - 50) };

        Vect2f startVel { Random::Float(-90, 90), Random::Float(-90, 90) };

        float speed = Random::Float(1, 5);
        float radius = Random::Float(1, 5);
        int points = Random::Int(3, 20);

        m_world.CreateEntity(CTransform(startPos, { 3, 3 }, 45),
            CMovement(speed),
            CRigidBody(startVel, radius, 0.1f, false),
            CCollider({ radius * 2, radius * 2 }, { 0, 0 }, false),
            CShape(points, Random::Color(), sf::Color::White, radius, 0));
    }
}
