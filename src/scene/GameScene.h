#pragma once

#include "ecs/World.h"
#include "ecs/query/Query.hpp"
#include "pathfinding/FlowFieldSystem.h"
#include "physics/CollisionSystem.h"
#include "physics/MovementSystem.h"
#include "scene/BaseScene.h"

class GameScene : public BaseScene
{
  private:
    MovementSystem m_movementSystem;
    CollisionSystem m_collisionSystem;
    FlowFieldSystem m_flowFieldSystem;

    Query* m_towerQuery;
    Query* m_enemyQuery;
    Query* m_collisionEventQueryPtr;
    void SpawnTestEntities();
    void SpawnEnemy(const Vect2f& spawnPos);

  public:
    GameScene(Vect2<uint16_t> windowSize);

    void OnStartPlay(World* worldPtr) override;
    void OnExitPlay(World* worldPtr) override;
    void OnChangeTo(World* worldPtr) override;
    void OnChangeFrom(World* worldPtr) override;
    void Update(size_t currentFrame, World* worldPtr, InputSystem& inputSystem) override;
};
