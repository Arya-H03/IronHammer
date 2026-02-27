#pragma once
#include "ecs/World.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"

class MovementSystem
{
  private:

    World* m_worldPtr;
    Query& movementQuery;

  public:

    MovementSystem(World* world) : m_worldPtr(world), movementQuery(world->Query<RequiredComponents<CTransform, CRigidBody, CMovement>>()) { }

    void HandleMovementSystem()
    {
        ZoneScoped;

        for (auto& archetype : movementQuery.GetMatchingArchetypes())
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
