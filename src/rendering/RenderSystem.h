#pragma once

#include "ecs/component/Components.hpp"
#include "ecs/system/ISystem.h"
#include "ecs/World.hpp"

#include <cstddef>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>

class RenderingSystem : ISetupSystem
{
private:
    Query* shapeQuery;
    Query* textQuery;
    Query* colliderQuery;
    Query* spriteQuery;

    // Flags
    bool m_canDrawText      = false;
    bool m_canDrawColliders = false;
    bool m_canDrawShapes    = true;
    bool m_canDrawSprites   = true;

    const size_t maxVerticesPerBatch = 10000;

private:
    size_t AddShapeToBatch(CShape& cshape, CTransform& ctransform, sf::VertexArray& batch);
    size_t AddColliderToBatch(CCollider& ccollider, CTransform& ctransform, sf::VertexArray& batch);
    void   AddSpriteToBatch(const CSprite& csprite, const CTransform& ctransform, sf::VertexArray& batch);

    void RenderShapes(sf::RenderTarget& renderTarget);
    void RenderColliders(sf::RenderTarget& renderTarget);
    void RenderText(sf::RenderTarget& renderTarget);
    void RenderSprites(sf::RenderTarget& renderTarget);

public:
    RenderingSystem(World* m_world);
    ~RenderingSystem();
    bool GetCanDrawText() const { return m_canDrawText; }
    bool GetCanDrawShapes() const { return m_canDrawShapes; }
    bool GetCanDrawColliders() const { return m_canDrawColliders; }
    bool GetCanDrawSprites() const { return m_canDrawSprites; }

    void SetCanDrawTest(bool val) { m_canDrawText = val; }
    void SetCanDrawShapes(bool val) { m_canDrawShapes = val; }
    void SetCanDrawColliders(bool val) { m_canDrawColliders = val; }
    void SetCanDrawSprites(bool val) { m_canDrawSprites = val; }

    void SetupSystem(World* newWorldPtr) override;

    void HandleRenderSystem(sf::RenderTarget& renderTarget);
};
