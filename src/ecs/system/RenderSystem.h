#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <cmath>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>
#include "ecs/component/Components.hpp"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "Tracy.hpp"

class RenderSystem
{

  private:
    sf::RenderWindow& m_window;
    ArchetypeRegistry& m_archetypeRegistry;
    sf::VertexArray m_shapeBatch;
    Query& renderShapesQuery;

    // Create a circle with CShape.points triangles
    // Each triangle is made out of 3 points
    void AddShapeToBatch(CShape& cshape, CTransform& ctransform)
    {
        const float innerRadius = cshape.radius - cshape.outlineThickness; // For main shape
        const float outerRadius = cshape.radius;                           // For outline

        const size_t points = cshape.points;
        const float rotationRad = ctransform.rotation * (M_PI / 180.f);

        sf::Vector2f center = {ctransform.position.x, ctransform.position.y};

        for (size_t i = 0; i < points; ++i)
        {
            float angle1 = (i * 2.f * M_PI) / points + rotationRad;
            float angle2 = ((i + 1) * 2.f * M_PI) / points + rotationRad;

            float a1Cos = std::cos(angle1);
            float a2Cos = std::cos(angle2);
            float a1Sin = std::sin(angle1);
            float a2Sin = std::sin(angle2);

            if (cshape.outlineThickness > 0)
            {
                sf::Vector2f outerPoint1 = {center.x + a1Cos * outerRadius, center.y + a1Sin * outerRadius};
                sf::Vector2f outerPoint2 = {center.x + a2Cos * outerRadius, center.y + a2Sin * outerRadius};

                m_shapeBatch.append(sf::Vertex(center, cshape.outlineColor));
                m_shapeBatch.append(sf::Vertex(outerPoint1, cshape.outlineColor));
                m_shapeBatch.append(sf::Vertex(outerPoint2, cshape.outlineColor));
            }

            sf::Vector2f innerPoint1 = {center.x + a1Cos * innerRadius, center.y + a1Sin * innerRadius};
            sf::Vector2f innerPoint2 = {center.x + a2Cos * innerRadius, center.y + a2Sin * innerRadius};

            m_shapeBatch.append(sf::Vertex(center, cshape.fillColor));
            m_shapeBatch.append(sf::Vertex(innerPoint1, cshape.fillColor));
            m_shapeBatch.append(sf::Vertex(innerPoint2, cshape.fillColor));
        }
    }

  public:
    RenderSystem(sf::RenderWindow& window, ArchetypeRegistry& archetypeRegistry)
        : m_window(window),
          m_archetypeRegistry(archetypeRegistry),
          renderShapesQuery(m_archetypeRegistry.CreateQuery<CShape, CTransform>())
    {
        m_shapeBatch.setPrimitiveType(sf::PrimitiveType::Triangles);
    }

    void HandleRenderSystem()
    {
        ZoneScoped;

        {
            ZoneScopedN("Clear");
            m_window.clear();
            m_shapeBatch.clear();
        }

        for (auto& archetype : renderShapesQuery.matchingArchetypes)
        {
            for (auto& chunk : archetype->GetChunks())
            {
                auto shapeCompRow = chunk.GetComponentRow<CShape>();
                auto transformCompRow = chunk.GetComponentRow<CTransform>();

                for (size_t i = 0; i < chunk.size; ++i)
                {
                    ZoneScopedN("Per Entity");

                    CShape& shapeComp = shapeCompRow[i];
                    CTransform& transformComp = transformCompRow[i];

                    {
                        ZoneScopedN("Construct Shape");
                        AddShapeToBatch(shapeComp, transformComp);
                    }
                }
            }
        }

        {
            ZoneScopedN("Draw Call");
            m_window.draw(m_shapeBatch);
        }

        ImGui::SFML::Render(m_window);
        m_window.display();
    }
};
