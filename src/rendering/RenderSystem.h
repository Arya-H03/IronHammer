#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <cstddef>
#include "ecs/World.hpp"

class RenderSystem
{
  private:

    World* m_worldPtr;

    Query* shapeQuery;
    Query* textQuery;
    Query* colliderQuery;

    // Flags
    bool m_canDrawText = false;
    bool m_canDrawColliders = false;
    bool m_canDrawShapes = true;

    const size_t maxVerticesPerBatch = 10000;

  private:

    size_t AddShapeToBatch(CShape& cshape, CTransform& ctransform, sf::VertexArray& batch);
    size_t AddColliderToBatch(CCollider& ccollider, CTransform& ctransform, sf::VertexArray& batch);

    void RenderShapes(sf::RenderTarget& renderTarget);
    void RenderColliders(sf::RenderTarget& renderTarget);
    void RenderText(sf::RenderTarget& renderTarget);

  public:

    RenderSystem(World* m_world);

    bool GetCanDrawText() const { return m_canDrawText; }
    bool GetCanDrawShapes() const { return m_canDrawShapes; }
    bool GetCanDrawColliders() const { return m_canDrawColliders; }

    void SetCanDrawTest(bool val) { m_canDrawText = val; }
    void SetCanDrawShapes(bool val) { m_canDrawShapes = val; }
    void SetCanDrawColliders(bool val) { m_canDrawColliders = val; }

    void ResetWorld(World* newWorldPtr);

    void HandleRenderSystem(sf::RenderTarget& renderTarget);
};
