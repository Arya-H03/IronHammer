#include "GameScene.h"
#include "core/utils/Random.hpp"
#include "ecs/World.hpp"

GameScene::GameScene(World* world,InputSystem& inputSystem, Vect2<uint16_t> windowSize)
    : BaseScene(world,inputSystem, windowSize), m_movementSystem(m_worldPtr), m_collisionSystem(m_worldPtr, m_windowSize)
{
}

void GameScene::OnEnter()
{
    SpawnTestEntities();
}

void GameScene::OnExit() { }

void GameScene::Update()
{
    m_inputManager.Update();
    m_collisionSystem.HandleCollisionSystem();
    m_movementSystem.HandleMovementSystem();

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

        m_worldPtr->CreateEntity(CTransform(startPos, { 3, 3 }, 45),
            CMovement(speed),
            CRigidBody(startVel, radius, 0.1f, false),
            CCollider({ radius * 2, radius * 2 }, { 0, 0 }, false),
            CShape(points, Random::Color(), sf::Color::White, radius, 0));
    }
}
