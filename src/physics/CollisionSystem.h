#pragma once
#include "Tracy.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Threadpool.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/query/Query.hpp"
#include "ecs/system/ISystem.h"
#include "editor/Viewport.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "game/GameComponents.hpp"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionCommon.h"
#include "physics/CollisionEventSystem.h"
#include "physics/CollisionResolutionSystem.h"
#include "physics/NarrowPhaseCollisionSystem.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

class CollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const uint8_t SUBSTEP_COUNT = 4;
    const Vect2f gravity = {0, 500};

    Vect2<uint16_t> m_windowSize;
    SolverBodies m_solverBodies;

    const size_t m_threadPoolCount = 3;
    ThreadPool m_threadPool;

    CollisionEventSystem m_collsionEventSystem;
    BroadPhaseCollisionSystem m_broadPhaseCollisionSystem;
    NarrowPhaseCollisionSystem m_narrowPhaseCollisionSystem;
    CollisionResolutionSystem m_collisionResolutionSystem;

    Query* m_updateCollisionQueryPtr;
    Query* m_updatePositionQueryPtr;

    void UpdateSolverBodyPositions(float dt)
    {
        ZoneScopedN("CollisionSystem/UpdatePositions");

        for (size_t i = 0; i < m_solverBodies.Count(); i++)
        {
            float velX = m_solverBodies.posX[i] - m_solverBodies.prePosX[i];
            float velY = m_solverBodies.posY[i] - m_solverBodies.prePosY[i];

            velX *= 0.99f;
            velY *= 0.99f;

            float newPosX = m_solverBodies.posX[i] + velX + gravity.x * dt * dt;
            float newPosY = m_solverBodies.posY[i] + velY + gravity.y * dt * dt;

            m_solverBodies.prePosX[i] = m_solverBodies.posX[i];
            m_solverBodies.prePosY[i] = m_solverBodies.posY[i];

            m_solverBodies.posX[i] = newPosX;
            m_solverBodies.posY[i] = newPosY;
        }
    }

    void ApplyViewportConstraints()
    {
        ZoneScopedN("CollisionSystem/CheckForScreenBorderCollision");

        for (size_t i = 0; i < m_solverBodies.Count(); i++)
        {
            // Top Edge
            if (m_solverBodies.posY[i] + m_solverBodies.colliderOffsetY[i] - m_solverBodies.colliderHalfSizeY[i] <= 0.0f)
            {
                m_solverBodies.posY[i] = m_solverBodies.colliderHalfSizeY[i] - m_solverBodies.colliderOffsetY[i];
            }
            // Bottom Edge
            else if (m_solverBodies.posY[i] + m_solverBodies.colliderOffsetY[i] + m_solverBodies.colliderHalfSizeY[i] >=
                     Viewport::GetSize().y)
            {
                m_solverBodies.posY[i] = Viewport::GetSize().y - m_solverBodies.colliderHalfSizeY[i] - m_solverBodies.colliderOffsetY[i];
            }

            // Left Edge
            if (m_solverBodies.posX[i] + m_solverBodies.colliderOffsetX[i] - m_solverBodies.colliderHalfSizeX[i] <= 0.0f)
            {
                m_solverBodies.posX[i] = m_solverBodies.colliderHalfSizeX[i] - m_solverBodies.colliderOffsetX[i];
            }
            // Right Edge
            else if (m_solverBodies.posX[i] + m_solverBodies.colliderOffsetX[i] + m_solverBodies.colliderHalfSizeX[i] >=
                     Viewport::GetSize().x)
            {
                m_solverBodies.posX[i] = Viewport::GetSize().x - m_solverBodies.colliderHalfSizeX[i] - m_solverBodies.colliderOffsetX[i];
            }
        }
    }

    void UpdateEntityPositions()
    {
        for (size_t i = 0; i < m_solverBodies.Count(); ++i)
        {
            m_solverBodies.transformPtrs[i]->position = Vect2f(m_solverBodies.posX[i], m_solverBodies.posY[i]);
            m_solverBodies.transformPtrs[i]->previousPosition = Vect2f(m_solverBodies.prePosX[i], m_solverBodies.prePosY[i]);
        }
    }

  public:
    void SetupSystem(World* worldPtr) override
    {
        m_updatePositionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CRigidBody, CFlowFieldAgent>>();
        m_updateCollisionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
        m_broadPhaseCollisionSystem.SetupSystem(worldPtr);
        m_collsionEventSystem.SetupSystem(worldPtr);

        m_solverBodies.Reserve(10000);
    }

    CollisionSystem(World* worldPtr, Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize), m_broadPhaseCollisionSystem(m_solverBodies, m_threadPool),
          m_narrowPhaseCollisionSystem(m_collsionEventSystem), m_threadPool(m_threadPoolCount)
    {
        SetupSystem(worldPtr);
        SystemDebuggerHub::Instance().GetCollsionDebugger().RegisterCollisionSystem(this);
    }

    ~CollisionSystem()
    {
        SystemDebuggerHub::Instance().GetCollsionDebugger().UnRegisterCollsionSystem();
    }

    void CreateSolverBodies(World* worldPtr)
    {
        m_solverBodies.Clear();

        m_updateCollisionQueryPtr->ForEachWithEntity<CTransform, CRigidBody, CCollider>(
            [&](Entity entity, CTransform& transform, CRigidBody& rigidBody, CCollider& collider)
            {
                m_solverBodies.AddSolverBody(entity, transform.position, transform.previousPosition, collider.offset, collider.halfSize,
                                             rigidBody.inverseMass, collider.mask, static_cast<uint32_t>(collider.layer), &transform);
            });
    }

    void HandleCollisionSystem(World* worldPtr, float dt)
    {
        SolverBodyPairs* solverBodyPairsPtr;
        CollisionResults* collisionResults;

        // m_collsionEventSystem.ClearCollisionEvents(worldPtr);

        float substepDt = dt / SUBSTEP_COUNT;
        for (size_t i = 0; i < SUBSTEP_COUNT; ++i)
        {
            UpdateSolverBodyPositions(substepDt);
            ApplyViewportConstraints();

            solverBodyPairsPtr = &m_broadPhaseCollisionSystem.HandleBroadPhaseCollisionSystem(worldPtr);
            collisionResults = &m_narrowPhaseCollisionSystem.ProccessPotentialCollisonPairs(worldPtr, m_solverBodies, *solverBodyPairsPtr);
            m_collisionResolutionSystem.ResolveCollisions(worldPtr, *collisionResults, m_solverBodies);
        }

        UpdateEntityPositions();

        // m_collsionEventSystem.HandleCollisionEvents(worldPtr, *collisionDataVector);
    }
};
