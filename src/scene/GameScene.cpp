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
    m_collisionEnterQuery = worldPtr->Query<RequiredComponents<CCollisionEnter>>();
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
        m_movementSystem.HandleMovementSystem();
    }
    {
        ZoneScopedN("GameScene/CollisionSystem");
        m_collisionSystem.HandleCollisionSystem(worldPtr);
    }


    float cd = 5;
    static float currentTime = 3;

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
        m_collisionEnterQuery->ForEach<CCollisionEnter>(
            [&](CCollisionEnter& collisionEnter)
            {
                Entity e1 = collisionEnter.entity1;
                Entity e2 = collisionEnter.entity2;

                if (worldPtr->HasComponent<CTower>(e1) && worldPtr->HasComponent<CEnemy>(e2))
                {
                    worldPtr->DestroyEntity(e2);
                }
                else if (worldPtr->HasComponent<CEnemy>(e1) && worldPtr->HasComponent<CTower>(e2))
                {
                    worldPtr->DestroyEntity(e1);
                }
            });
    }
}
void GameScene::SpawnTestEntities()
{
    size_t count = 5000;

    for (size_t i = 0; i < count; ++i)
    {
        Vect2f startPos{Random::Float(20, Viewport::GetSize().x - 20), Random::Float(20, Viewport::GetSize().y - 20)};
        Vect2f velocity = Random::Vect2f({-1, 1}, {-1, 1});

        float speed = Random::Float(50, 100);
        float radius = 10;
        float bounce = 1;
        float mass = Random::Float(1, 100);

        m_worldPtr->CreateEntityNoReturn(CTransform(startPos, {1, 1}, 0), CMovement(speed), CRigidBody(velocity, mass, bounce, false),
                                         CCollider({radius, radius}, {0, 0}, Layer::Enemy, ~0u, false),CFlowFieldAgent(),CEnemy(),
                                         CSprite("Square", Vect2f(radius, radius), sf::IntRect({0, 0}, {256, 256}), Random::Color()));
    }
}
