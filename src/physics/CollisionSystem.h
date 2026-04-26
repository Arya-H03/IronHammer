#pragma once
#include "Tracy.hpp"
#include "backward.hpp"
#include "core/CoreComponents.hpp"
#include "core/utils/Colors.h"
#include "core/utils/Time.h"
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
#include "rendering/RenderingComponents.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <cstdint>
#include <cstdlib>
#include <vector>

class CollisionSystem : public ISetupSystem
{
    friend class CollisionDebugger;

  private:
    const uint8_t SUBSTEP_COUNT = 4;
    const Vect2f gravity = {50, 200};

    Vect2<uint16_t> m_windowSize;

    CollisionEventSystem m_collsionEventSystem;
    BroadPhaseCollisionSystem m_broadPhaseCollisionSystem;
    NarrowPhaseCollisionSystem m_narrowPhaseCollisionSystem;
    CollisionResolutionSystem m_collisionResolutionSystem;

    Query* m_updateCollisionQueryPtr;
    Query* m_updatePositionQueryPtr;

    void UpdatePositions(float dt)
    {
        m_updatePositionQueryPtr->ForEach<CTransform, CRigidBody, CMovement, CFlowFieldAgent>(
            [&](CTransform& transform, CRigidBody& rigidBody, CMovement& movement, CFlowFieldAgent& flowFieldAgent)
            {
                if (rigidBody.isStatic) return;

                Vect2f velocity = transform.position - transform.previousPosition;
                velocity *= .99f;
                transform.previousPosition = transform.position;
                transform.position += velocity + flowFieldAgent.flowDir * 100 * dt * dt;
            });
    }

    void CheckForScreenBorderCollision()
    {
        ZoneScopedN("CollisionSystem/CheckForScreenBorderCollision");

        m_updateCollisionQueryPtr->ForEach<CTransform, CRigidBody, CCollider>(
            [](CTransform& transform, CRigidBody& rigidBody, CCollider& collider)
            {
                if (rigidBody.isStatic) return;

                if (transform.position.y + collider.offset.y - collider.halfSize.y <= 0.0f)
                {
                    transform.position.y = collider.halfSize.y - collider.offset.y;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.y *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }
                else if (transform.position.y + collider.offset.y + collider.halfSize.y >= Viewport::GetSize().y)
                {
                    transform.position.y = Viewport::GetSize().y - collider.halfSize.y - collider.offset.y;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.y *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }

                if (transform.position.x + collider.offset.x - collider.halfSize.x <= 0.0f)
                {
                    transform.position.x = collider.halfSize.x - collider.offset.x;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.x *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }
                else if (transform.position.x + collider.offset.x + collider.halfSize.x >= Viewport::GetSize().x)
                {
                    transform.position.x = Viewport::GetSize().x - collider.halfSize.x - collider.offset.x;
                    Vect2f velocity = transform.position - transform.previousPosition;
                    velocity.x *= -rigidBody.bounciness;
                    transform.previousPosition = transform.position - velocity;
                }
            });
    }


  public:
    void SetupSystem(World* worldPtr) override
    {
        m_updatePositionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CRigidBody, CFlowFieldAgent>>();
        m_updateCollisionQueryPtr = worldPtr->Query<RequiredComponents<CTransform, CCollider, CRigidBody>>();
        m_broadPhaseCollisionSystem.SetupSystem(worldPtr);
        m_collsionEventSystem.SetupSystem(worldPtr);
        // worldPtr->CreateEntityNoReturn(CTransform({500, 500}, {1, 1}, 0),
        //                                CSprite("Circle", {500, 500}, sf::IntRect{{0, 0}, {256, 256}}, Colors::ColdSteelBlue_SFML));
    }

    CollisionSystem(World* worldPtr, Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize), m_broadPhaseCollisionSystem({1500, 1500}), m_narrowPhaseCollisionSystem(m_collsionEventSystem)
    {
        SetupSystem(worldPtr);
        SystemDebuggerHub::Instance().GetCollsionDebugger().RegisterCollisionSystem(this);
    }

    ~CollisionSystem()
    {
        SystemDebuggerHub::Instance().GetCollsionDebugger().UnRegisterCollsionSystem();
    }

    void HandleCollisionSystem(World* worldPtr, float dt)
    {
        ZoneScoped;


        std::vector<PotentialCollisionPair>* potentialCollisionPairVector;
        std::vector<CollisionCorrectionData>* collisionDataVector;
        m_collsionEventSystem.ClearCollisionEvents(worldPtr);

        float substepDt = dt / SUBSTEP_COUNT;
        for (size_t i = 0; i < SUBSTEP_COUNT; ++i)
        {
            UpdatePositions(substepDt);

            CheckForScreenBorderCollision();
            potentialCollisionPairVector = &m_broadPhaseCollisionSystem.HandleBroadPhaseCollisionSystem(worldPtr);
            collisionDataVector = &m_narrowPhaseCollisionSystem.ProccessPotentialCollisonPairs(worldPtr, *potentialCollisionPairVector);
            m_collisionResolutionSystem.ResolveCollisions(*collisionDataVector);
        }

        m_collsionEventSystem.HandleCollisionEvents(worldPtr, *collisionDataVector);
    }
};
