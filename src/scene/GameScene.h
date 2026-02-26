#pragma once

#include "scene/BaseScene.h"
#include "physics/MovementSystem.h"
#include "physics/CollisionSystem.h"
#include "gui/GuiSystem.h"

class GameScene : public BaseScene
{
  private:

    MovementSystem m_movementSystem;
    CollisionSystem m_collisionSystem;
    GuiSystem m_guiSystem;

    void SpawnTestEntities();


  public:

    GameScene(sf::RenderWindow& window, InputSystem& inputSystem, Vect2<uint16_t> windowSize);

    void OnEnter() override;
    void OnExit() override;
    void Update() override;

};
