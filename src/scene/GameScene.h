#pragma once

#include "ecs/World.hpp"
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

    GameScene(Vect2<uint16_t> windowSize);

    void OnStartPlay(World* worldPtr) override;
    void OnExitPlay(World* worldPtr) override;
    void OnChangeTo(World* worldPtr) override;
    void OnChangeFrom(World* worldPtr) override;
    void Update(World* worldPtr,InputSystem& inputSystem) override;
};
