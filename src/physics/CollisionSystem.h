#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>
#include <cstdlib>
#include "core/utils/Vect2.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"
#include "ecs/entity/EntityManager.hpp"
#include "physics/BroadPhaseCollisionSystem.h"
#include "physics/CollisionDebugger.h"

class CollisionSystem
{
    friend class CollisionDebugger;

  private:

    Vect2<uint16_t> m_windowSize;
    EntityManager& m_entityManager;
    ArchetypeRegistry& m_ArchetypeRegistry;
    BroadPhaseCollisionSystem m_broadPhaseCollisionSystem;
    CollisionDebugger m_collisionDebugger;
    Query& collisionQuery;

  public:

    const CollisionDebugger& GetCollsionDebugger() const { return m_collisionDebugger; }

    CollisionSystem(EntityManager& entityManager, ArchetypeRegistry& archetypeRegistry, Vect2<uint16_t> windowSize)
        : m_windowSize(windowSize)
        , m_entityManager(entityManager)
        , m_ArchetypeRegistry(archetypeRegistry)
        , m_broadPhaseCollisionSystem(entityManager, m_ArchetypeRegistry, m_windowSize)
        , m_collisionDebugger(m_broadPhaseCollisionSystem)
        , collisionQuery(m_ArchetypeRegistry.CreateQuery<RequiredComponents<CTransform,CCollider,CMovement>>())
    {
    }

    void HandleCollisionSystem()
    {
        ZoneScoped;

        // for (size_t a1 = 0; a1 < m_matchingArchetypes.size(); ++a1)
        // {
        //     auto& archetype1 = *m_matchingArchetypes[a1];
        //     auto& chunks1 = archetype1.GetChunks();

        //     for (size_t c1 = 0; c1 < chunks1.size(); ++c1)
        //     {
        //         auto& chunk1 = chunks1[c1];

        //         auto transformRow1 = chunk1.GetComponentRow<CTransform>();
        //         auto movementRow1 = chunk1.GetComponentRow<CMovement>();
        //         auto colliderRow1 = chunk1.GetComponentRow<CCollider>();

        //         for (size_t i1 = 0; i1 < chunk1.size; ++i1)
        //         {
        //             CTransform& t1 = transformRow1[i1];
        //             CMovement& m1 = movementRow1[i1];
        //             CCollider& c1c = colliderRow1[i1];

        //             // ------------------------------------
        //             // Window bounds collision
        //             // ------------------------------------

        //             if (t1.position.y - c1c.boundingBox.bounds.y <= 0
        //                 || t1.position.y + c1c.boundingBox.bounds.y >= m_window.getSize().y)
        //             {
        //                 m1.velocity.y *= -1;
        //             }

        //             if (t1.position.x - c1c.boundingBox.bounds.x <= 0
        //                 || t1.position.x + c1c.boundingBox.bounds.x >= m_window.getSize().x)
        //             {
        //                 m1.velocity.x *= -1;
        //             }

        //             // ------------------------------------
        //             // Entity-Entity collisions
        //             // ------------------------------------

        //             for (size_t a2 = a1; a2 < m_matchingArchetypes.size(); ++a2)
        //             {
        //                 auto& archetype2 = *m_matchingArchetypes[a2];
        //                 auto& chunks2 = archetype2.GetChunks();

        //                 for (size_t c2 = 0; c2 < chunks2.size(); ++c2)
        //                 {
        //                     auto& chunk2 = chunks2[c2];

        //                     auto transformRow2 = chunk2.GetComponentRow<CTransform>();
        //                     auto movementRow2 = chunk2.GetComponentRow<CMovement>();
        //                     auto colliderRow2 = chunk2.GetComponentRow<CCollider>();

        //                     size_t startIndex = 0;

        //                     if (a1 == a2 && c1 == c2) startIndex = i1 + 1;

        //                     for (size_t i2 = startIndex; i2 < chunk2.size; ++i2)
        //                     {
        //                         CTransform& t2 = transformRow2[i2];
        //                         CMovement& m2 = movementRow2[i2];
        //                         CCollider& c2c = colliderRow2[i2];

        //                         float dx = std::abs(t1.position.x - t2.position.x);
        //                         float dy = std::abs(t1.position.y - t2.position.y);

        //                         float overlapX = c1c.boundingBox.bounds.x + c2c.boundingBox.bounds.x;
        //                         float overlapY = c1c.boundingBox.bounds.y + c2c.boundingBox.bounds.y;

        //                         if (dx <= overlapX && dy <= overlapY)
        //                         {
        //                             m1.velocity.x *= -1;
        //                             m1.velocity.y *= -1;

        //                             m2.velocity.x *= -1;
        //                             m2.velocity.y *= -1;
        //                         }
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }

        {
            ZoneScopedN("Collision Check Against Bounds");
            for (auto& archetype : collisionQuery.GetMatchingArchetypes())
            {
                for (auto& chunk : archetype->GetChunks())
                {
                    auto transformCompRow = chunk.GetComponentRow<CTransform>();
                    auto movementCompRow = chunk.GetComponentRow<CMovement>();
                    auto colliderCompRow = chunk.GetComponentRow<CCollider>();

                    for (size_t i = 0; i < chunk.size; ++i)
                    {
                        CTransform& transformComp = transformCompRow[i];
                        CMovement& movementComp = movementCompRow[i];
                        CCollider& colliderComp = colliderCompRow[i];

                        if (transformComp.position.y + colliderComp.offset.y - colliderComp.size.y <= 0
                            || transformComp.position.y + colliderComp.offset.y + colliderComp.size.y >= m_windowSize.y)
                        {
                            movementComp.velocity.y *= -1;
                        }

                        if (transformComp.position.x + colliderComp.offset.x - colliderComp.size.x <= 0
                            || transformComp.position.x + colliderComp.offset.x + colliderComp.size.x >= m_windowSize.x)
                        {
                            movementComp.velocity.x *= -1;
                        }
                    }
                }
            }
        }

        {
            ZoneScopedN("Broad Phase System");
            m_broadPhaseCollisionSystem.HandleBroadPhaseCollisionSystem();
        }
    }
};
