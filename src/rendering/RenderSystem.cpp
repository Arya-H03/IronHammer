#include "RenderSystem.h"

#include "Tracy.hpp"
#include "assets/AssetManager.h"
#include "core/FrameRateHandler.h"
#include "core/utils/Colors.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "physics/PhysicsComponents.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Vector2.hpp>
#include <cmath>
#include <imgui-SFML.h>

RenderingSystem::RenderingSystem(World* worldPtr)
{
    SetupSystem(worldPtr);
    SystemDebuggerHub::Instance().GetRenderignSystemDebugger().RegisterRenderingSystem(this);
}

RenderingSystem::~RenderingSystem()
{
    SystemDebuggerHub::Instance().GetRenderignSystemDebugger().UnRegisterRenderingSystem();
}

void RenderingSystem::SetupSystem(World* newWorldPtr)
{
    shapeQuery = newWorldPtr->Query<RequiredComponents<CShape, CTransform>, ExcludedComponents<CNotDrawable>>();
    textQuery = newWorldPtr->Query<RequiredComponents<CText, CTransform>, ExcludedComponents<CNotDrawable>>();
    colliderQuery = newWorldPtr->Query<RequiredComponents<CCollider, CTransform>, ExcludedComponents<CNotDrawable>>();
    spriteQuery = newWorldPtr->Query<RequiredComponents<CSprite, CTransform, CRigidBody>, ExcludedComponents<CNotDrawable>>();
}

size_t RenderingSystem::AddShapeToBatch(CShape& cshape, CTransform& ctransform, sf::VertexArray& batch)
{
    const float innerRadius = cshape.radius - cshape.outlineThickness;
    const float outerRadius = cshape.radius;
    const float rotationRad = ctransform.rotation * (M_PI / 180.f);
    const size_t points = cshape.points;

    size_t verticesAdded = 0;

    sf::Vector2f center = {ctransform.position.x, ctransform.position.y};

    for (size_t i = 0; i < points; ++i)
    {
        float angle1 = (i * 2.f * M_PI) / points + rotationRad + M_PI / 4;
        float angle2 = ((i + 1) * 2.f * M_PI) / points + rotationRad + M_PI / 4;

        float a1Cos = std::cos(angle1), a2Cos = std::cos(angle2);
        float a1Sin = std::sin(angle1), a2Sin = std::sin(angle2);

        if (cshape.outlineThickness > 0)
        {
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
    const float halfWidth = ccollider.size.x * 0.5f;
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

void RenderingSystem::AddSpriteToBatch(const CSprite& sprite, const CTransform& transform, const CRigidBody& rigidBody,
                                       sf::VertexArray& batch)
{
    ZoneScopedN("Add Sprite to Batch");

    float width = sprite.size.x * transform.scale.x;
    float height = sprite.size.y * transform.scale.y;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    Vect2f center = transform.previousPosition + ((transform.position - transform.previousPosition) * FrameRateHandler::GetRenderAlpha());

    float rad = transform.rotation * (float)M_PI / 180.0f;
    float cosRad = std::cos(rad);
    float sinRad = std::sin(rad);

    auto Rotate = [&](float x, float y) { return sf::Vector2f(center.x + x * cosRad - y * sinRad, center.y + x * sinRad + y * cosRad); };

    sf::Vector2 topLeft = Rotate(-halfWidth, -halfHeight);
    sf::Vector2 topRight = Rotate(halfWidth, -halfHeight);
    sf::Vector2 bottomRight = Rotate(halfWidth, halfHeight);
    sf::Vector2 bottomLeft = Rotate(-halfWidth, halfHeight);

    float u1 = sprite.textureRect.position.x;
    float v1 = sprite.textureRect.position.y;
    float u2 = sprite.textureRect.position.x + sprite.textureRect.size.x;
    float v2 = sprite.textureRect.position.y + sprite.textureRect.size.y;

    sf::Vector2f uvTopLeft(u1, v1);
    sf::Vector2f uvTopRight(u2, v1);
    sf::Vector2f uvBottomRight(u2, v2);
    sf::Vector2f uvBottomLeft(u1, v2);

    batch.append(sf::Vertex(topLeft, sprite.color, uvTopLeft));
    batch.append(sf::Vertex(topRight, sprite.color, uvTopRight));
    batch.append(sf::Vertex(bottomRight, sprite.color, uvBottomRight));

    batch.append(sf::Vertex(topLeft, sprite.color, uvTopLeft));
    batch.append(sf::Vertex(bottomRight, sprite.color, uvBottomRight));
    batch.append(sf::Vertex(bottomLeft, sprite.color, uvBottomLeft));
}

void RenderingSystem::RenderSprites(sf::RenderTarget& renderTarget)
{
    static sf::VertexArray batch(sf::PrimitiveType::Triangles);
    batch.clear();

    const sf::Texture* currentTexture = nullptr;

    spriteQuery->ForEach<CSprite, CTransform, CRigidBody>(
        [&](CSprite& csprite, CTransform& ctransform, CRigidBody& rigidBody)
        {
            // Batch
            if (currentTexture != csprite.texturePtr)
            {
                renderTarget.draw(batch, currentTexture);
                batch.clear();
                currentTexture = csprite.texturePtr;
            }

            AddSpriteToBatch(csprite, ctransform, rigidBody, batch);
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

    shapeQuery->ForEach<CShape, CTransform>(
        [&](CShape& shape, CTransform& transform)
        {
            size_t newVertices = AddShapeToBatch(shape, transform, batch);
            verticesInBatch += newVertices;

            if (verticesInBatch >= maxVerticesPerBatch)
            {
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

    colliderQuery->ForEach<CCollider, CTransform>(
        [&](CCollider& collider, CTransform& transform)
        {
            size_t newVertices = AddColliderToBatch(collider, transform, batch);
            verticesInBatch += newVertices;

            if (verticesInBatch >= maxVerticesPerBatch)
            {
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
    textQuery->ForEach<CText, CTransform>(
        [&](CText& textComp, CTransform& transformComp)
        {
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
