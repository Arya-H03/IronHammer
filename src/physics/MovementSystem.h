#pragma once
#include "ecs/World.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"
#include "ecs/system/ISystem.h"

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

        for (auto& archetype : m_movementQuery->GetMatchingArchetypes())
        {
            for (auto& chunk : archetype->GetChunks())
            {
                auto transformCompRow = chunk.GetComponentRow<CTransform>();
                auto movementCompRow = chunk.GetComponentRow<CMovement>();
                auto rigidBodyCompRow = chunk.GetComponentRow<CRigidBody>();

                for (size_t i = 0; i < chunk.size; ++i)
                {
                    CTransform& transformComp = transformCompRow[i];
                    CMovement& movementComp = movementCompRow[i];
                    CRigidBody& rigidBodyComp = rigidBodyCompRow[i];

                    Vect2f velocityNormalized = rigidBodyComp.velocity.Normalize();
                    rigidBodyComp.previousPosition = transformComp.position;
                    transformComp.position += velocityNormalized * movementComp.speed;
                }
            }
        }
    }
};
