#pragma once
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"

class MovementSystem
{
  private:

    ArchetypeRegistry& m_archetypeRegistry;
    Query& movementQuery;

  public:

    MovementSystem(ArchetypeRegistry& archetypeRegistry)
        : m_archetypeRegistry(archetypeRegistry)
        , movementQuery(m_archetypeRegistry.CreateQuery<RequiredComponents<CTransform, CMovement>>())
    {
    }

    void HandleMovementSystem()
    {
        ZoneScoped;

        for (auto& archetype : movementQuery.GetMatchingArchetypes())
        {
            for (auto& chunk : archetype->GetChunks())
            {
                auto transformCompRow = chunk.GetComponentRow<CTransform>();
                auto movementCompRow = chunk.GetComponentRow<CMovement>();

                for (size_t i = 0; i < chunk.size; ++i)
                {
                    CTransform& transformComp = transformCompRow[i];
                    CMovement& movementComp = movementCompRow[i];

                    Vect2f velocityNormalized = movementComp.velocity.Normalize();
                    transformComp.position += velocityNormalized * movementComp.speed;
                }
            }
        }
    }
};
