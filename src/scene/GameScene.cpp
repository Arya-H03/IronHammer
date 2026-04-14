#include "GameScene.h"

#include "core/CoreComponents.hpp"
#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "editor/Viewport.h"
#include "game/GameComponents.hpp"
#include "input/InputSystem.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Window/Joystick.hpp>
#include <cstdint>
#include <string>

GameScene::GameScene(Vect2<uint16_t> windowSize) : BaseScene(windowSize), m_collisionSystem(m_windowSize)
{
}

void GameScene::OnStartPlay(World* worldPtr)
{
    m_isPlaying = true;
    m_isPaused = false;
    m_worldPtr = worldPtr;

    m_movementSystem.SetupSystem(worldPtr);
    m_collisionSystem.SetupSystem(worldPtr);
    m_flowFieldSystem.SetupSystem(worldPtr);

    m_towerQuery = worldPtr->Query<RequiredComponents<CTower, CTransform>>();
    m_enemyQuery = worldPtr->Query<RequiredComponents<CEnemy, CTransform>>();
    m_collisionEventQueryPtr = worldPtr->Query<RequiredComponents<CCollisionEvent, CTower>>();
}

void GameScene::OnExitPlay(World* worldPtr)
{
    m_isPlaying = false;
    m_worldPtr = nullptr;
    m_isPaused = false;

    SystemDebuggerHub::Instance().GetFlowFieldDebugger().ResetDebugger();
}

void GameScene::OnChangeTo(World* worldPtr)
{
}

void GameScene::OnChangeFrom(World* worldPtr)
{
    m_isPlaying = false;
}

void GameScene::Update(size_t currentFrame, World* worldPtr, InputSystem& inputSystem)
{
    if (m_isPaused) return;

    m_inputManager.Update(inputSystem);

    {
        ZoneScopedN("GameScene/FlowFieldUpdate");
        m_flowFieldSystem.UpdateFlowAgents();
    }
    {
        ZoneScopedN("GameScene/UpdateFlowAgents");
        m_flowFieldSystem.UpdateFlowAgents();
    }
    {
        ZoneScopedN("GameScene/MovementSystem");
        // m_movementSystem.HandleMovementSystem();
    }
    {
        ZoneScopedN("GameScene/CollisionSystem");
        m_collisionSystem.HandleCollisionSystem(worldPtr, Time::DeltaTime());
    }


    float cd = 2;
    static float currentTime = 1;

    {
        ZoneScopedN("GameScene/SpawnTestEntities");
        if (currentTime >= cd)
        {
            SpawnTestEntities();
            currentTime = 0;
        }
    }
    // Lemao refactor later
    // Probably a BFS or a simple 1D loop
    currentTime += Time::DeltaTime();

    {
        ZoneScopedN("GameScene/Towe&EnemyCollisionEvent");
        m_collisionEventQueryPtr->ForEach<CCollisionEvent>(
            [&](CCollisionEvent& collisionEvent)
            {
                for (size_t i = 0; i < collisionEvent.enterEvents.count; ++i)
                {
                    Entity collisionEntity = collisionEvent.enterEvents.list[i];
                    if (worldPtr->HasComponent<CEnemy>(collisionEntity))
                    {
                        worldPtr->DestroyEntity(collisionEntity);
                    }
                }
            });
    }
}
void GameScene::SpawnTestEntities()
{
    for (int i = 10; i <= Viewport::GetSize().x - 10; i += 16)
    {
        Vect2f startPos{(float)i, 10};
        Vect2f velocity = Random::Vect2f({-1, 1}, {0, 100});

        float speed = Random::Float(25, 50);
        float radius = Random::Float(10, 16);
        float bounce = 0;
        float mass = 1;

        m_worldPtr->CreateEntityNoReturn(CTransform(startPos, {1, 1}, 0), CMovement(speed), CRigidBody({0, 0}, {0, 0}, mass, bounce, false),
                                         CCollider({radius, radius}, {0, 0}, Layer::Enemy, ~0u, false), CFlowFieldAgent(), CEnemy(),
                                         CSprite("Circle", Vect2f(radius, radius), sf::IntRect({0, 0}, {256, 256}), Random::Color()));
    }
    for (int i = 10; i <= Viewport::GetSize().x - 10; i += 16)
    {
        Vect2f startPos{(float)i, (float)Viewport::GetSize().y - 10};
        Vect2f velocity = Random::Vect2f({-1, 1}, {-1, 1});

        float speed = Random::Float(25, 50);
        float radius = Random::Float(10, 16);
        float bounce = 0;
        float mass = 1;

        m_worldPtr->CreateEntityNoReturn(CTransform(startPos, {1, 1}, 0), CMovement(speed), CRigidBody({0, 0}, {0, 0}, mass, bounce, false),
                                         CCollider({radius, radius}, {0, 0}, Layer::Enemy, ~0u, false), CFlowFieldAgent(), CEnemy(),
                                         CSprite("Circle", Vect2f(radius, radius), sf::IntRect({0, 0}, {256, 256}), Random::Color()));
    }
    for (int i = 10; i <= Viewport::GetSize().y - 10; i += 16)
    {
        Vect2f startPos{0, (float)i};
        Vect2f velocity = Random::Vect2f({-1, 1}, {-1, 1});


        float speed = Random::Float(25, 50);
        float radius = Random::Float(10, 16);
        float bounce = 0;
        float mass = 1;

        m_worldPtr->CreateEntityNoReturn(CTransform(startPos, {1, 1}, 0), CMovement(speed), CRigidBody({0, 0}, {0, 0}, mass, bounce, false),
                                         CCollider({radius, radius}, {0, 0}, Layer::Enemy, ~0u, false), CFlowFieldAgent(), CEnemy(),
                                         CSprite("Circle", Vect2f(radius, radius), sf::IntRect({0, 0}, {256, 256}), Random::Color()));
    }
    for (int i = 10; i <= Viewport::GetSize().y - 10; i += 16)
    {
        Vect2f startPos{(float)Viewport::GetSize().x - 10, float(i)};
        Vect2f velocity = Random::Vect2f({-1, 1}, {-1, 1});

        float speed = Random::Float(25, 50);
        float radius = Random::Float(14, 14);
        float bounce = 0;
        float mass = 1;

        m_worldPtr->CreateEntityNoReturn(CTransform(startPos, {1, 1}, 0), CMovement(speed), CRigidBody({0, 0}, {0, 0}, mass, bounce, false),
                                         CCollider({radius, radius}, {0, 0}, Layer::Enemy, ~0u, false), CEnemy(), CFlowFieldAgent(),
                                         CSprite("Circle", Vect2f(radius, radius), sf::IntRect({0, 0}, {256, 256}), Random::Color()));
    }
}
