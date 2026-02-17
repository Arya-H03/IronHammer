#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include "ecs/system/BaseSystem.h"
#include "Tracy.hpp"

class MovementSystem: public BaseSystem
{
    public:

    MovementSystem()
    {
        MakeSignatureMask<CTransform,CMovement>();
    }

    void HandleMovementSystem()
    {
        ZoneScoped;

        for (auto& archetype : m_matchingArchetypes)
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
