#include "RenderSystem.h"

#include "assets/AssetManager.h"
#include "assets/FontManager.h"
#include "core/utils/Colors.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/Components.hpp"
#include "ecs/World.hpp"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "Tracy.hpp"

#include <cmath>
#include <imgui-SFML.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Vector2.hpp>

RenderingSystem::RenderingSystem(World* world)
    : shapeQuery(world->Query<RequiredComponents<CShape, CTransform>, ExcludedComponents<CNotDrawable>>()),
      textQuery(world->Query<RequiredComponents<CText, CTransform>, ExcludedComponents<CNotDrawable>>()),
      colliderQuery(world->Query<RequiredComponents<CCollider, CTransform>, ExcludedComponents<CNotDrawable>>()),
      spriteQuery(world->Query<RequiredComponents<CSprite, CTransform>, ExcludedComponents<CNotDrawable>>())
{
    SystemDebuggerHub::Get().GetRenderignSystemDebugger().RegisterRenderingSystem(this);
}

RenderingSystem::~RenderingSystem() { SystemDebuggerHub::Get().GetRenderignSystemDebugger().UnRegisterRenderingSystem(); }

void RenderingSystem::SetupSystem(World* newWorldPtr)
{
    shapeQuery    = newWorldPtr->Query<RequiredComponents<CShape, CTransform>, ExcludedComponents<CNotDrawable>>();
    textQuery     = newWorldPtr->Query<RequiredComponents<CText, CTransform>, ExcludedComponents<CNotDrawable>>();
    colliderQuery = newWorldPtr->Query<RequiredComponents<CCollider, CTransform>, ExcludedComponents<CNotDrawable>>();
    spriteQuery   = newWorldPtr->Query<RequiredComponents<CSprite, CTransform>, ExcludedComponents<CNotDrawable>>();
}

size_t RenderingSystem::AddShapeToBatch(CShape& cshape, CTransform& ctransform, sf::VertexArray& batch)
{
    const float  innerRadius = cshape.radius - cshape.outlineThickness;
    const float  outerRadius = cshape.radius;
    const float  rotationRad = ctransform.rotation * (M_PI / 180.f);
    const size_t points      = cshape.points;

    size_t verticesAdded = 0;

    sf::Vector2f center = {ctransform.position.x, ctransform.position.y};

    for (size_t i = 0; i < points; ++i) {
        float angle1 = (i * 2.f * M_PI) / points + rotationRad + M_PI / 4;
        float angle2 = ((i + 1) * 2.f * M_PI) / points + rotationRad + M_PI / 4;

        float a1Cos = std::cos(angle1), a2Cos = std::cos(angle2);
        float a1Sin = std::sin(angle1), a2Sin = std::sin(angle2);

        if (cshape.outlineThickness > 0) {
            sf::Vector2f outer1{center.x + a1Cos * outerRadius, center.y + a1Sin * outerRadius};
            sf::Vector2f outer2{center.x + a2Cos * outerRadius, center.y + a2Sin * outerRadius};

            batch.append(sf::Vertex(center, cshape.outlineColor));
            batch.append(sf::Vertex(outer1, cshape.outlineColor));
            batch.append(sf::Vertex(outer2, cshape.outlineColor));
            verticesAdded += 3;
        }

        sf::Vector2f inner1{center.x + a1Cos * innerRadius, center.y + a1Sin * innerRadius};
        sf::Vector2f inner2{center.x + a2Cos * innerRadius, center.y + a2Sin * innerRadius};

        batch.append(sf::Vertex(center, cshape.fillColor));
        batch.append(sf::Vertex(inner1, cshape.fillColor));
        batch.append(sf::Vertex(inner2, cshape.fillColor));
        verticesAdded += 3;
    }

    return verticesAdded;
}

size_t RenderingSystem::AddColliderToBatch(CCollider& ccollider, CTransform& ctransform, sf::VertexArray& batch)
{
    const float halfWidth  = ccollider.size.x * 0.5f;
    const float halfHeight = ccollider.size.y * 0.5f;

    sf::Vector2f center(ctransform.position.x + ccollider.offset.x, ctransform.position.y + ccollider.offset.y);

    sf::Vector2f topLeft{center.x - halfWidth, center.y - halfHeight};
    sf::Vector2f topRight{center.x + halfWidth, center.y - halfHeight};
    sf::Vector2f bottomRight{center.x + halfWidth, center.y + halfHeight};
    sf::Vector2f bottomLeft{center.x - halfWidth, center.y + halfHeight};

    batch.append(sf::Vertex(topLeft, Colors::OxidizedGreen_SFML));
    batch.append(sf::Vertex(topRight, Colors::OxidizedGreen_SFML));

    batch.append(sf::Vertex(topRight, Colors::OxidizedGreen_SFML));
    batch.append(sf::Vertex(bottomRight, Colors::OxidizedGreen_SFML));

    batch.append(sf::Vertex(bottomRight, Colors::OxidizedGreen_SFML));
    batch.append(sf::Vertex(bottomLeft, Colors::OxidizedGreen_SFML));

    batch.append(sf::Vertex(bottomLeft, Colors::OxidizedGreen_SFML));
    batch.append(sf::Vertex(topLeft, Colors::OxidizedGreen_SFML));

    return 8;
}

void RenderingSystem::AddSpriteToBatch(const CSprite& csprite, const CTransform& ctransform, sf::VertexArray& batch)
{
    ZoneScopedN("Add Sprite to Batch");

    float width      = csprite.size.x * ctransform.scale.x;
    float height     = csprite.size.y * ctransform.scale.y;
    float halfWidth  = width / 2.0f;
    float halfHeight = height / 2.0f;

    Vect2f center = ctransform.position;

    float rad    = ctransform.rotation * (float)M_PI / 180.0f;
    float cosRad = std::cos(rad);
    float sinRad = std::sin(rad);

    auto Rotate = [&](float x, float y) {
        return sf::Vector2f(center.x + x * cosRad - y * sinRad, center.y + x * sinRad + y * cosRad);
    };

    sf::Vector2 topLeft     = Rotate(-halfWidth, -halfHeight);
    sf::Vector2 topRight    = Rotate(halfWidth, -halfHeight);
    sf::Vector2 bottomRight = Rotate(halfWidth, halfHeight);
    sf::Vector2 bottomLeft  = Rotate(-halfWidth, halfHeight);

    float u1 = csprite.textureRect.position.x;
    float v1 = csprite.textureRect.position.y;
    float u2 = csprite.textureRect.position.x + csprite.textureRect.size.x;
    float v2 = csprite.textureRect.position.y + csprite.textureRect.size.y;

    sf::Vector2f uvTopLeft(u1, v1);
    sf::Vector2f uvTopRight(u2, v1);
    sf::Vector2f uvBottomRight(u2, v2);
    sf::Vector2f uvBottomLeft(u1, v2);

    batch.append(sf::Vertex(topLeft, csprite.color, uvTopLeft));
    batch.append(sf::Vertex(topRight, csprite.color, uvTopRight));
    batch.append(sf::Vertex(bottomRight, csprite.color, uvBottomRight));

    batch.append(sf::Vertex(topLeft, csprite.color, uvTopLeft));
    batch.append(sf::Vertex(bottomRight, csprite.color, uvBottomRight));
    batch.append(sf::Vertex(bottomLeft, csprite.color, uvBottomLeft));
}

void RenderingSystem::RenderSprites(sf::RenderTarget& renderTarget)
{
    static sf::VertexArray batch(sf::PrimitiveType::Triangles);
    batch.clear();

    const sf::Texture* currentTexture = nullptr;

    spriteQuery->ForEach<CSprite, CTransform>([&](CSprite& csprite, CTransform& ctransform) {
        // Batch
        if (currentTexture != csprite.texture) {
            renderTarget.draw(batch, currentTexture);
            batch.clear();
            currentTexture = csprite.texture;
        }

        AddSpriteToBatch(csprite, ctransform, batch);
        // Individual
        //  sf::Sprite spriteToDraw(*csprite.texture);
        //  spriteToDraw.setTextureRect(csprite.textureRect);
        //  spriteToDraw.setColor(csprite.color);
        //  spriteToDraw.setPosition({ctransform.position.x, ctransform.position.y});
        //  spriteToDraw.setOrigin({ctransform.position.x * 0.5f, ctransform.position.y *
        //  0.5f}); spriteToDraw.setScale({csprite.size.x / csprite.textureRect.size.x,
        //  csprite.size.y / csprite.textureRect.size.y}); renderTarget.draw(spriteToDraw);
    });

    renderTarget.draw(batch, currentTexture);
}

void RenderingSystem::RenderShapes(sf::RenderTarget& renderTarget)
{
    static sf::VertexArray batch(sf::PrimitiveType::Triangles);
    batch.clear();
    batch.resize(0);

    size_t verticesInBatch = 0;

    shapeQuery->ForEach<CShape, CTransform>([&](CShape& shape, CTransform& transform) {
        size_t newVertices = AddShapeToBatch(shape, transform, batch);
        verticesInBatch += newVertices;

        if (verticesInBatch >= maxVerticesPerBatch) {
            ZoneScopedN("Draw Shape Batch");
            renderTarget.draw(batch);
            batch.clear();
            verticesInBatch = 0;
        }
    });

    if (verticesInBatch > 0) renderTarget.draw(batch);
}

void RenderingSystem::RenderColliders(sf::RenderTarget& renderTarget)
{
    sf::VertexArray batch(sf::PrimitiveType::Lines);

    size_t verticesInBatch = 0;

    colliderQuery->ForEach<CCollider, CTransform>([&](CCollider& collider, CTransform& transform) {
        size_t newVertices = AddColliderToBatch(collider, transform, batch);
        verticesInBatch += newVertices;

        if (verticesInBatch >= maxVerticesPerBatch) {
            renderTarget.draw(batch);
            batch.clear();
            verticesInBatch = 0;
        }
    });

    if (verticesInBatch > 0) renderTarget.draw(batch);
}


// Fix me: Texts render upside down
void RenderingSystem::RenderText(sf::RenderTarget& renderTarget)
{
    textQuery->ForEach<CText, CTransform>([&](CText& textComp, CTransform& transformComp) {
        sf::Text text{*AssetManager::Instance().LoadFont("Default")};
        text.setString(textComp.content);
        text.setFillColor(textComp.textColor);
        text.setCharacterSize(textComp.fontSize);
        text.setPosition({transformComp.position.x - textComp.offset.x, transformComp.position.y - textComp.offset.y});

        renderTarget.draw(text);
    });
}

void RenderingSystem::HandleRenderSystem(sf::RenderTarget& renderTarget)
{

    {
        ZoneScopedN("Render Shapes");
        if (m_canDrawShapes) RenderShapes(renderTarget);
    }

    {
        ZoneScopedN("Render Colliders");
        if (m_canDrawColliders) RenderColliders(renderTarget);
    }

    {
        ZoneScopedN("Render Sprites");
        if (m_canDrawSprites) RenderSprites(renderTarget);
    }
    {
        ZoneScopedN("Render Text");
        if (m_canDrawText) RenderText(renderTarget);
    }
}
