#include "GameScene.h"

#include "core/utils/Debug.h"
#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "ecs/World.hpp"
#include "editor/Viewport.h"
#include "input/InputSystem.h"
#include "nlohmann/json.hpp"

#include <cstdint>
#include <string>

GameScene::GameScene(Vect2<uint16_t> windowSize)
    : BaseScene(windowSize), m_collisionSystem(m_windowSize)
{
}

void GameScene::OnStartPlay(World* worldPtr)
{
    m_isPlaying = true;
    m_isPaused  = false;
    m_worldPtr  = worldPtr;

    m_movementSystem.SetupSystem(worldPtr);
    m_collisionSystem.SetupSystem(worldPtr);
    m_towerQuery = worldPtr->Query<RequiredComponents<CTower, CTransform>>();
    m_enemyQuery = worldPtr->Query<RequiredComponents<CEnemy, CTransform>>();

    SpawnTestEntities();
}

void GameScene::OnExitPlay(World* worldPtr)
{
    m_isPlaying = false;
    m_worldPtr  = nullptr;
    m_isPaused  = false;
}

void GameScene::OnChangeTo(World* worldPtr) {}

void GameScene::OnChangeFrom(World* worldPtr) { m_isPlaying = false; }

void GameScene::Update(size_t currentFrame, World* worldPtr, InputSystem& inputSystem)
{
    if (m_isPaused) return;

    m_inputManager.Update(inputSystem);
    m_movementSystem.HandleMovementSystem();
    m_collisionSystem.HandleCollisionSystem(worldPtr);

    if (currentFrame % 60 == 0)
    {
        for (auto& archetype : m_towerQuery->GetMatchingArchetypes())
        {
            for (auto& chunk : archetype->GetChunks())
            {
                CTransform* towerTransformRow = chunk.GetComponentRow<CTransform>();
                for (size_t i = 0; i < chunk.size; ++i)
                {
                    CTransform& towerTransform = towerTransformRow[i];

                    CTransform* closestEnemyTransform;
                    size_t      closestDistance = SIZE_MAX;
                    for (auto& archetype : m_enemyQuery->GetMatchingArchetypes())
                    {
                        for (auto& chunk : archetype->GetChunks())
                        {
                            CTransform* enemyTransformRow = chunk.GetComponentRow<CTransform>();
                            for (size_t j = 0; j < chunk.size; ++j)
                            {
                                CTransform& enemyTransform = enemyTransformRow[j];
                                float       dist =
                                    towerTransform.position.Distance(enemyTransform.position);
                                if (dist < closestDistance)
                                {
                                    closestDistance       = dist;
                                    closestEnemyTransform = &enemyTransform;
                                }
                            }
                        }
                    }
                    if (closestEnemyTransform)
                    {
                        Vect2f vel = closestEnemyTransform->position - towerTransform.position;
                        m_worldPtr->CreateEntity(
                            CTransform(towerTransform.position, {0.5, 0.5}, 0), CMovement(5),
                            CRigidBody(vel, 1, 0.1f, false), CCollider({25, 25}, {0, 0}, false),
                            CSprite("Square", Vect2f(25, 25), sf::IntRect({0, 0}, {256, 256}),
                                    Random::Color()));
                    }
                }
            }
        }
    }
}
void GameScene::SpawnTestEntities()
{
    size_t count = 0;

    for (size_t i = 0; i < count; ++i)
    {
        Vect2f startPos{Random::Float(Viewport::GetPosition().x,
                                      Viewport::GetPosition().x + Viewport::GetSize().x * 0.5f),
                        Random::Float(Viewport::GetPosition().y,
                                      Viewport::GetPosition().y + Viewport::GetSize().y + 0.5f)};
        Vect2f startVel{Random::Float(-90, 90), Random::Float(-90, 90)};

        float speed  = Random::Float(1, 5);
        float radius = Random::Float(1, 5);

        m_worldPtr->CreateEntity(CTransform(startPos, {3, 3}, 45), CMovement(speed),
                                 CRigidBody(startVel, radius, 0.1f, true),
                                 CCollider({radius * 2, radius * 2}, {0, 0}, false),
                                 CSprite("Square", Vect2f(radius, radius),
                                         sf::IntRect({0, 0}, {256, 256}), Random::Color()));
    }
}
