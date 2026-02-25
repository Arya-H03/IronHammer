#include "RenderSystem.h"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/System/Vector2.hpp>
#include <imgui-SFML.h>
#include <cmath>
#include "assets/FontManager.h"
#include "Tracy.hpp"
#include "core/utils/Colors.h"

RenderSystem::RenderSystem(World& world, sf::RenderWindow& window)
    : m_window(window)
    , m_world(world)
    , shapeQuery(world.Query<RequiredComponents<CShape, CTransform>, ExcludedComponents<CNotDrawable>>())
    , textQuery(world.Query<RequiredComponents<CText, CTransform>, ExcludedComponents<CNotDrawable>>())
    , colliderQuery(world.Query<RequiredComponents<CCollider, CTransform>, ExcludedComponents<CNotDrawable>>())
{
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
        float angle1 = (i * 2.f * M_PI) / points + rotationRad;
        float angle2 = ((i + 1) * 2.f * M_PI) / points + rotationRad;

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

void RenderSystem::RenderShapes()
{
    static sf::VertexArray batch(sf::PrimitiveType::Triangles);
    batch.clear();
    batch.resize(0);

    size_t verticesInBatch = 0;

    for (auto& archetype : shapeQuery.GetMatchingArchetypes())
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
                    m_window.draw(batch);
                    batch.clear();
                    verticesInBatch = 0;
                }
            }
        }
    }
    if (verticesInBatch > 0) m_window.draw(batch);
}

void RenderSystem::RenderColliders()
{
    sf::VertexArray batch(sf::PrimitiveType::Lines);

    size_t verticesInBatch = 0;

    for (auto& archetype : colliderQuery.GetMatchingArchetypes())
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
                    m_window.draw(batch);
                    batch.clear();
                    verticesInBatch = 0;
                }
            }
        }
    }

    if (verticesInBatch > 0) m_window.draw(batch);
}

void RenderSystem::RenderText()
{
    for (auto& archetype : textQuery.GetMatchingArchetypes())
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
                        m_window.draw(text);
                    }
                }
            }
        }
    }
}

void RenderSystem::HandleRenderSystem()
{
    m_window.clear();

    if (m_canDrawShapes) RenderShapes();
    if (m_canDrawColliders) RenderColliders();
    if (m_canDrawText) RenderText();

    ImGui::SFML::Render(m_window);
    m_window.display();
}
