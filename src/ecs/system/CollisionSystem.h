#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdlib>
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"

class CollisionSystem
{
  private:
    const sf::RenderWindow& m_window;
    ArchetypeRegistry& m_ArchetypeRegistry;
    Query& collisionQuery;

  public:
    CollisionSystem(const sf::RenderWindow& window, ArchetypeRegistry& archetypeRegistry)
        : m_window(window),
          m_ArchetypeRegistry(archetypeRegistry),
          collisionQuery(m_ArchetypeRegistry.CreateQuery<CTransform, CCollider, CMovement>())
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

        for (auto& archetype : collisionQuery.matchingArchetypes)
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

                    if (transformComp.position.y - colliderComp.boundingBox.bounds.y <= 0
                        || transformComp.position.y + colliderComp.boundingBox.bounds.y >= m_window.getSize().y)
                    {
                        movementComp.velocity.y *= -1;
                    }

                    if (transformComp.position.x - colliderComp.boundingBox.bounds.x <= 0
                        || transformComp.position.x + colliderComp.boundingBox.bounds.x >= m_window.getSize().x)
                    {
                        movementComp.velocity.x *= -1;
                    }
                }
            }
        }
    }
};
