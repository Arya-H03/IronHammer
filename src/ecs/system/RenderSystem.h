#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <cstddef>

#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/component/Components.hpp"

class RenderSystem
{
  private:

    sf::RenderWindow& m_window;
    ArchetypeRegistry& m_archetypeRegistry;

    Query& shapeQuery;
    Query& textQuery;
    Query& colliderQuery;

    // Flags
    bool m_canDrawText = true;
    bool m_canDrawColliders = true;
    bool m_canDrawShapes = true;

    const size_t maxVerticesPerBatch = 1000;

  private:

    size_t AddShapeToBatch(CShape& cshape, CTransform& ctransform, sf::VertexArray& batch);
    size_t AddColliderToBatch(CCollider& ccollider, CTransform& ctransform, sf::VertexArray& batch);

    void RenderShapes();
    void RenderColliders();
    void RenderText();

  public:

    bool GetCanDrawText() const { return m_canDrawText; }
    bool GetCanDrawShapes() const { return m_canDrawShapes; }
    bool GetCanDrawColliders() const { return m_canDrawColliders; }

    void SetCanDrawTest(bool val) { m_canDrawText = val; }
    void SetCanDrawShapes(bool val) { m_canDrawShapes = val; }
    void SetCanDrawColliders(bool val) { m_canDrawColliders = val; }

    RenderSystem(sf::RenderWindow& window, ArchetypeRegistry& archetypeRegistry);

    void HandleRenderSystem();
};
