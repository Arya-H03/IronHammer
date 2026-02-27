#pragma once

#include "scene/BaseScene.h"
#include "physics/MovementSystem.h"
#include "physics/CollisionSystem.h"

class GameScene : public BaseScene
{
  private:

    MovementSystem m_movementSystem;
    CollisionSystem m_collisionSystem;

    void SpawnTestEntities();

  public:

    GameScene(World* world,InputSystem& inputSystem,Vect2<uint16_t> windowSize);

    void OnEnter() override;
    void OnExit() override;
    void Update() override;
};
