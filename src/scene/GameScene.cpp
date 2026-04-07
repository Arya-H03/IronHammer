#include "GameScene.h"

#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "editor/Viewport.h"
#include "game/GameComponents.hpp"
#include "input/InputSystem.h"

#include <cstdint>
#include <string>

GameScene::GameScene(Vect2<uint16_t> windowSize) : BaseScene(windowSize), m_collisionSystem(m_windowSize) {}

void GameScene::OnStartPlay(World* worldPtr)
{
    m_isPlaying = true;
    m_isPaused  = false;
    m_worldPtr  = worldPtr;

    m_movementSystem.SetupSystem(worldPtr);
    m_collisionSystem.SetupSystem(worldPtr);
    m_flowFieldSystem.SetupSystem(worldPtr);
    m_towerQuery = worldPtr->Query<RequiredComponents<CTower, CTransform>>();
    m_enemyQuery = worldPtr->Query<RequiredComponents<CEnemy, CTransform>>();

    m_flowFieldSystem.UpdateFlowField(worldPtr);
    SpawnTestEntities();
}

void GameScene::OnExitPlay(World* worldPtr)
{
    m_isPlaying = false;
    m_worldPtr  = nullptr;
    m_isPaused  = false;

    SystemDebuggerHub::Instance().GetFlowFieldDebugger().ResetDebugger();
}

void GameScene::OnChangeTo(World* worldPtr) {}

void GameScene::OnChangeFrom(World* worldPtr) { m_isPlaying = false; }

void GameScene::Update(size_t currentFrame, World* worldPtr, InputSystem& inputSystem)
{
    if (m_isPaused) return;

    m_inputManager.Update(inputSystem);
    m_flowFieldSystem.UpdateFlowAgents();
    m_movementSystem.HandleMovementSystem();
    m_collisionSystem.HandleCollisionSystem(worldPtr);

    float cd = 2;

    static float currentTime = 0;

    // Lemao refactor later
    // Probably a BFS or a simple 1D loop
    if (currentTime >= cd) {
        m_towerQuery->ForEach<CTransform>([&](CTransform& towerTransform) {
            CTransform* closestEnemyTransform;
            size_t      closestDistance = SIZE_MAX;

            m_enemyQuery->ForEach<CTransform>([&](CTransform& enemyTransform) {
                float dist = towerTransform.position.Distance(enemyTransform.position);
                if (dist < closestDistance) {
                    closestDistance       = dist;
                    closestEnemyTransform = &enemyTransform;
                }
            });

            if (closestEnemyTransform) {
                Vect2f vel = closestEnemyTransform->position - towerTransform.position;
                m_worldPtr->CreateEntityNoReturn(
                    CTransform(towerTransform.position, {1, 1}, 0), CMovement(150), CRigidBody(vel, 1, 0.1f, false),
                    CCollider({25, 25}, {0, 0}, Layer::Default, ~0u, false),
                    CSprite("Square", Vect2f(25, 25), sf::IntRect({0, 0}, {256, 256}), Random::Color()));

                currentTime = 0;
            }
        });
    }
    currentTime += Time::DeltaTime();

    // for (auto& archetype : m_enterCollisionQuery->GetMatchingArchetypes()) {
    //     for (auto& chunk : archetype->GetChunks()) {
    //         CCollisionEnter* collisionEnterRow = chunk.GetComponentRow<CCollisionEnter>();
    //         for (size_t i = 0; i < chunk.size; ++i) {
    //             CCollisionEnter& collisionEnter = collisionEnterRow[i];

    //             Entity e1 = collisionEnter.entity1;
    //             Entity e2 = collisionEnter.entity2;

    //             if ((worldPtr->HasComponent<CTower>(e1) && worldPtr->HasComponent<CEnemy>(e2)) ||
    //                 (worldPtr->HasComponent<CEnemy>(e1) && worldPtr->HasComponent<CTower>(e2))) {
    //                 worldPtr->DestroyEntity(e1);
    //                 worldPtr->DestroyEntity(e2);
    //             }
    //         }
    //     }
    // }
}
void GameScene::SpawnTestEntities()
{
    size_t count = 0;

    for (size_t i = 0; i < count; ++i) {
        Vect2f startPos{Random::Float(0, Viewport::GetSize().x), Random::Float(0, Viewport::GetSize().y)};
        Vect2f startVel{Random::Float(-1, 1), Random::Float(-1, 1)};

        float speed  = Random::Float(100, 150);
        float radius = Random::Float(2, 10);
        float bounce = Random::Float(0, 1);
        float mass   = Random::Float(1, 100);

        m_worldPtr->CreateEntityNoReturn(
            CTransform(startPos, {1, 1}, 0), CMovement(speed), CRigidBody(startVel, mass, bounce, false),
            CCollider({radius, radius}, {0, 0}, Layer::Default, ~0u, false),
            CSprite("Square", Vect2f(radius, radius), sf::IntRect({0, 0}, {256, 256}), Random::Color()));
    }
}
