#pragma once
#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "ecs/World.h"
#include "ecs/system/ISystem.h"
#include "physics/PhysicsComponents.hpp"

class MovementSystem : public ISetupSystem
{
  private:
    Query* m_movementQuery;

  public:
    MovementSystem() = default;

    void SetupSystem(World* worldPtr) override
    {
        m_movementQuery = worldPtr->Query<RequiredComponents<CTransform, CRigidBody, CMovement>>();
    }
    void HandleMovementSystem()
    {
        ZoneScoped;

        m_movementQuery->ForEach<CTransform, CMovement, CRigidBody>(
            [](CTransform& transformComp, CMovement& movementComp, CRigidBody& rigidBodyComp)
            {
                Vect2f velocityNormalized = rigidBodyComp.velocity.Normalize();
                transformComp.previousPosition = transformComp.position;
                transformComp.position += velocityNormalized * movementComp.speed * Time::DeltaTime();
            });
    }
};
