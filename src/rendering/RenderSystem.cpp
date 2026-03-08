#include "RenderSystem.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/System/Vector2.hpp>
#include <imgui-SFML.h>
#include <cmath>
#include "assets/FontManager.h"
#include "Tracy.hpp"
#include "core/utils/Colors.h"
#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"

RenderSystem::RenderSystem(World* world)
    : m_worldPtr(world)
    , shapeQuery(m_worldPtr->Query<RequiredComponents<CShape, CTransform>, ExcludedComponents<CNotDrawable>>())
    , textQuery(m_worldPtr->Query<RequiredComponents<CText, CTransform>, ExcludedComponents<CNotDrawable>>())
    , colliderQuery(m_worldPtr->Query<RequiredComponents<CCollider, CTransform>, ExcludedComponents<CNotDrawable>>())
    , spriteQuery(m_worldPtr->Query<RequiredComponents<CSprite, CTransform>, ExcludedComponents<CNotDrawable>>())
{
}

void RenderSystem::ResetWorld(World* newWorldPtr)
{
    m_worldPtr = newWorldPtr;
    shapeQuery = m_worldPtr->Query<RequiredComponents<CShape, CTransform>, ExcludedComponents<CNotDrawable>>();
    textQuery = m_worldPtr->Query<RequiredComponents<CText, CTransform>, ExcludedComponents<CNotDrawable>>();
    colliderQuery = m_worldPtr->Query<RequiredComponents<CCollider, CTransform>, ExcludedComponents<CNotDrawable>>();
    spriteQuery = m_worldPtr->Query<RequiredComponents<CSprite, CTransform>, ExcludedComponents<CNotDrawable>>();
}

size_t RenderSystem::AddShapeToBatch(CShape& cshape, CTransform& ctransform, sf::VertexArray& batch)
{
    const float innerRadius = cshape.radius - cshape.outlineThickness;
    const float outerRadius = cshape.radius;
    const float rotationRad = ctransform.rotation * (M_PI / 180.f);
    const size_t points = cshape.points;

    size_t verticesAdded = 0;

    sf::Vector2f center = { ctransform.position.x, ctransform.position.y };

    for (size_t i = 0; i < points; ++i)
    {
        float angle1 = (i * 2.f * M_PI) / points + rotationRad + M_PI / 4;
        float angle2 = ((i + 1) * 2.f * M_PI) / points + rotationRad + M_PI / 4;

        float a1Cos = std::cos(angle1), a2Cos = std::cos(angle2);
        float a1Sin = std::sin(angle1), a2Sin = std::sin(angle2);

        // Outline
        if (cshape.outlineThickness > 0)
        {
            sf::Vector2f outer1 { center.x + a1Cos * outerRadius, center.y + a1Sin * outerRadius };
            sf::Vector2f outer2 { center.x + a2Cos * outerRadius, center.y + a2Sin * outerRadius };

            batch.append(sf::Vertex(center, cshape.outlineColor));
            batch.append(sf::Vertex(outer1, cshape.outlineColor));
            batch.append(sf::Vertex(outer2, cshape.outlineColor));
            verticesAdded += 3;
        }

        // Fill
        sf::Vector2f inner1 { center.x + a1Cos * innerRadius, center.y + a1Sin * innerRadius };
        sf::Vector2f inner2 { center.x + a2Cos * innerRadius, center.y + a2Sin * innerRadius };

        batch.append(sf::Vertex(center, cshape.fillColor));
        batch.append(sf::Vertex(inner1, cshape.fillColor));
        batch.append(sf::Vertex(inner2, cshape.fillColor));
        verticesAdded += 3;
    }

    return verticesAdded;
}

size_t RenderSystem::AddColliderToBatch(CCollider& ccollider, CTransform& ctransform, sf::VertexArray& batch)
{
    const float halfWidth = ccollider.size.x * 0.5f;
    const float halfHeight = ccollider.size.y * 0.5f;

    sf::Vector2f center(ctransform.position.x + ccollider.offset.x, ctransform.position.y + ccollider.offset.y);

    // 4 corners
    sf::Vector2f topLeft { center.x - halfWidth, center.y - halfHeight };
    sf::Vector2f topRight { center.x + halfWidth, center.y - halfHeight };
    sf::Vector2f bottomRight { center.x + halfWidth, center.y + halfHeight };
    sf::Vector2f bottomLeft { center.x - halfWidth, center.y + halfHeight };

    // Each pair = one line segment
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

void RenderSystem::AddSpriteToBatch(const CSprite& csprite, const CTransform& ctransform, sf::VertexArray& batch)
{
    float width = csprite.size.x * ctransform.scale.x;
    float height = csprite.size.y * ctransform.scale.y;
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    Vect2f center = ctransform.position;

    float rad = ctransform.rotation * (float) M_PI / 180.0f;
    float cosRad = std::cos(rad);
    float sinRad = std::sin(rad);

    auto ApplyRotation = [&](float x, float y) { return sf::Vector2f(center.x + x * cosRad - y * sinRad, center.y + x * sinRad + y * cosRad); };

    sf::Vector2 point1 = ApplyRotation(-halfWidth, -halfHeight);
    sf::Vector2 point2 = ApplyRotation(halfWidth, -halfHeight);
    sf::Vector2 point3 = ApplyRotation(halfWidth, halfHeight);
    sf::Vector2 point4 = ApplyRotation(-halfWidth, halfHeight);

    float u1 = csprite.textureRect.position.x;
    float v1 = csprite.textureRect.position.y;
    float u2 = csprite.textureRect.position.x + csprite.textureRect.size.x;
    float v2 = csprite.textureRect.position.y + csprite.textureRect.size.y;

    sf::Vector2f uvTopLeft(u1, v1);
    sf::Vector2f uvTopRight(u2, v1);
    sf::Vector2f uvBottomRight(u2, v2);
    sf::Vector2f uvBottomLeft(u1, v2);

    // First triangle
    batch.append(sf::Vertex(point1, csprite.color, uvTopLeft));
    batch.append(sf::Vertex(point2, csprite.color, uvTopRight));
    batch.append(sf::Vertex(point3, csprite.color, uvBottomRight));

    // Second triangle
    batch.append(sf::Vertex(point1, csprite.color, uvTopLeft));
    batch.append(sf::Vertex(point3, csprite.color, uvBottomRight));
    batch.append(sf::Vertex(point4, csprite.color, uvBottomLeft));
}

void RenderSystem::RenderSprites(sf::RenderTarget& renderTarget)
{
    static sf::VertexArray batch(sf::PrimitiveType::Triangles);
    batch.clear();

    const sf::Texture* currentTexture = nullptr;

    for (auto& archetype : spriteQuery->GetMatchingArchetypes())
    {
        for (auto& chunk : archetype->GetChunks())
        {
            ZoneScopedN("Per Entity Render Sprites");

            auto spriteCompRow = chunk.GetComponentRow<CSprite>();
            auto transformCompRow = chunk.GetComponentRow<CTransform>();

            for (size_t i = 0; i < chunk.size; ++i)
            {
                const CSprite& csprite = spriteCompRow[i];
                const CTransform& ctransform = transformCompRow[i];

                if (currentTexture != csprite.texture)
                {
                    renderTarget.draw(batch, currentTexture);
                    batch.clear();
                    currentTexture = csprite.texture;
                }

                AddSpriteToBatch(csprite, ctransform, batch);
            }
        }
    }
    renderTarget.draw(batch, currentTexture);
}

void RenderSystem::RenderShapes(sf::RenderTarget& renderTarget)
{
    static sf::VertexArray batch(sf::PrimitiveType::Triangles);
    batch.clear();
    batch.resize(0);

    size_t verticesInBatch = 0;

    for (auto& archetype : shapeQuery->GetMatchingArchetypes())
    {
        for (auto& chunk : archetype->GetChunks())
        {
            ZoneScopedN("Per Entity Render Shapes");

            auto shapeCompRow = chunk.GetComponentRow<CShape>();
            auto transformCompRow = chunk.GetComponentRow<CTransform>();

            for (size_t i = 0; i < chunk.size; ++i)
            {
                {
                    ZoneScopedN("Add Shape To Batch");

                    CShape& shape = shapeCompRow[i];
                    CTransform& transform = transformCompRow[i];

                    size_t newVertices = AddShapeToBatch(shape, transform, batch);
                    verticesInBatch += newVertices;
                }

                if (verticesInBatch >= maxVerticesPerBatch)
                {
                    ZoneScopedN("Draw Shape Batch");
                    renderTarget.draw(batch);
                    batch.clear();
                    verticesInBatch = 0;
                }
            }
        }
    }
    if (verticesInBatch > 0) renderTarget.draw(batch);
}

void RenderSystem::RenderColliders(sf::RenderTarget& renderTarget)
{
    sf::VertexArray batch(sf::PrimitiveType::Lines);

    size_t verticesInBatch = 0;

    for (auto& archetype : colliderQuery->GetMatchingArchetypes())
    {
        for (auto& chunk : archetype->GetChunks())
        {
            // ZoneScopedN("Per Entity Render Colliders");

            auto colliderCompRow = chunk.GetComponentRow<CCollider>();
            auto transformCompRow = chunk.GetComponentRow<CTransform>();

            for (size_t i = 0; i < chunk.size; ++i)
            {
                {
                    // ZoneScopedN("Add Collider To Batch");

                    CCollider& collider = colliderCompRow[i];
                    CTransform& transform = transformCompRow[i];

                    size_t newVertices = AddColliderToBatch(collider, transform, batch);
                    verticesInBatch += newVertices;
                }

                if (verticesInBatch >= maxVerticesPerBatch)
                {
                    // ZoneScopedN("Draw Collider Batch");
                    renderTarget.draw(batch);
                    batch.clear();
                    verticesInBatch = 0;
                }
            }
        }
    }

    if (verticesInBatch > 0) renderTarget.draw(batch);
}

void RenderSystem::RenderText(sf::RenderTarget& renderTarget)
{
    for (auto& archetype : textQuery->GetMatchingArchetypes())
    {
        for (auto& chunk : archetype->GetChunks())
        {
            auto textCompRow = chunk.GetComponentRow<CText>();
            auto transformCompRow = chunk.GetComponentRow<CTransform>();

            for (size_t i = 0; i < chunk.size; ++i)
            {
                // ZoneScopedN("Per Entity RenderText");

                CText& textComp = textCompRow[i];
                CTransform& transformComp = transformCompRow[i];
                {
                    // ZoneScopedN("Construct Text");

                    sf::Text text(FontManager::GetFont());
                    text.setString(textComp.content);
                    text.setFillColor(textComp.textColor);
                    text.setCharacterSize(textComp.fontSize);
                    text.setPosition({ transformComp.position.x - textComp.offset.x, transformComp.position.y - textComp.offset.y });

                    {
                        // ZoneScopedN("Draw Text");
                        renderTarget.draw(text);
                    }
                }
            }
        }
    }
}

void RenderSystem::HandleRenderSystem(sf::RenderTarget& renderTarget)
{
    // m_window.clear();

    if (m_canDrawShapes) RenderShapes(renderTarget);
    if (m_canDrawColliders) RenderColliders(renderTarget);
    if (m_canDrawText) RenderText(renderTarget);
    RenderSprites(renderTarget);

    // ImGui::SFML::Render(m_window);
    // m_window.display();
}
