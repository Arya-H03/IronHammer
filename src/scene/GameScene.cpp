#include "GameScene.h"

#include "core/CoreComponents.hpp"
#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "editor/Viewport.h"
#include "game/GameComponents.hpp"
#include "input/InputManager.h"
#include "input/InputSystem.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <cstdint>
#include <string>

GameScene::GameScene(Vect2<uint16_t> windowSize) : BaseScene(windowSize)
{
}

void GameScene::OnStartPlay(World* worldPtr)
{
    m_isPlaying = true;
    m_isPaused = false;
    m_worldPtr = worldPtr;

    m_movementSystem.SetupSystem(worldPtr);
    m_flowFieldSystem.SetupSystem(worldPtr);

    m_towerQuery = worldPtr->Query<RequiredComponents<CTower, CTransform>>();
    m_enemyQuery = worldPtr->Query<RequiredComponents<CEnemy, CTransform>>();
    m_collisionEventQueryPtr = worldPtr->Query<RequiredComponents<CCollisionEvent, CTower>>();

    m_inputManager.CreateInputAction("SpawnEnemy", sf::Keyboard::Key::S, InputTrigger::Pressed, [&]() { SpawnTestEntities(); });
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

    float cd = 0.1;
    static float currentTime = 0;

    if (currentTime >= cd)
    {
        SpawnTestEntities();
        currentTime = 0;
    }
    // // Lemao refactor later
    // // Probably a BFS or a simple 1D loop
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

void GameScene::SpawnEnemy(const Vect2f& spawnPos, const Vect2f& velocity)
{
    float speed = Random::Float(50, 75);
    float radius = Random::Float(22, 22);
    float bounce = 0.1f;
    float mass = radius;

    m_worldPtr->CreateEntityNoReturn(CTransform(spawnPos, {1, 1}, 0), CMovement(speed), CRigidBody(velocity, {0, 10}, mass, bounce, false),
                                     CCollider({radius, radius}, {0, 0}, Layer::Enemy, ~0u, false), CEnemy(), CFlowFieldAgent(),
                                     CSprite("Circle", Vect2f(radius, radius), sf::IntRect({0, 0}, {256, 256}), Random::Color()));
}

void GameScene::SpawnTestEntities()
{
    SpawnEnemy({25, 25}, {250, 250});

    SpawnEnemy({(float)Viewport::GetSize().x - 25, 25}, {-250, 250});
    // for (int i = 10; i <= Viewport::GetSize().x - 10; i += 24)
    // {
    //     Vect2f startPos{(float)i, 10};
    //     SpawnEnemy(startPos);
    // }
    // for (int i = 10; i <= Viewport::GetSize().x - 10; i += 24)
    // {
    //     Vect2f startPos{(float)i, (float)Viewport::GetSize().y - 10};
    //     SpawnEnemy(startPos);
    // }
    // for (int i = 10; i <= Viewport::GetSize().y - 10; i += 24)
    // {
    //     Vect2f startPos{0, (float)i};
    //     SpawnEnemy(startPos);
    // }
    // for (int i = 10; i <= Viewport::GetSize().y - 10; i += 24)
    // {
    //     Vect2f startPos{(float)Viewport::GetSize().x - 10, float(i)};
    //     SpawnEnemy(startPos);
    // }
}
