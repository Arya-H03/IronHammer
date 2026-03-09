#include "GameScene.h"
#include "core/utils/Random.hpp"
#include "ecs/World.hpp"
#include "editor/Viewport.h"
#include "input/InputSystem.h"

GameScene::GameScene(Vect2<uint16_t> windowSize) : BaseScene(windowSize), m_collisionSystem(m_windowSize) { }

void GameScene::OnStartPlay(World* worldPtr)
{
    m_isPlaying = true;
    m_isPaused = false;
    m_worldPtr = worldPtr;

    m_movementSystem.SetupSystem(worldPtr);
    m_collisionSystem.SetupSystem(worldPtr);

    SpawnTestEntities();
}

void GameScene::OnExitPlay(World* worldPtr)
{
    m_isPlaying = false;
    m_worldPtr = nullptr;
    m_isPaused = false;
}

void GameScene::OnChangeTo(World* worldPtr) { }

void GameScene::OnChangeFrom(World* worldPtr) { m_isPlaying = false; }

void GameScene::Update(World* worldPtr, InputSystem& inputSystem)
{
    if (m_isPaused) return;

    m_inputManager.Update(inputSystem);
    m_movementSystem.HandleMovementSystem();
    m_collisionSystem.HandleCollisionSystem(worldPtr);
}

void GameScene::SpawnTestEntities()
{
    size_t count = 0;

    for (size_t i = 0; i < count; ++i)
    {
        Vect2f startPos { Random::Float(Viewport::GetPosition().x, Viewport::GetPosition().x + Viewport::GetSize().x * 0.5f),
            Random::Float(Viewport::GetPosition().y, Viewport::GetPosition().y + Viewport::GetSize().y + 0.5f) };
        Vect2f startVel { Random::Float(-90, 90), Random::Float(-90, 90) };

        float speed = Random::Float(1, 5);
        float radius = Random::Float(1, 5);

        m_worldPtr->CreateEntity(CTransform(startPos, { 3, 3 }, 45),
            CMovement(speed),
            CRigidBody(startVel, radius, 0.1f, true),
            CCollider({ radius * 2, radius * 2 }, { 0, 0 }, false),
            CSprite("Square", Vect2f(radius, radius), sf::IntRect({ 0, 0 }, { 256, 256 }), Random::Color()));
    }
}
