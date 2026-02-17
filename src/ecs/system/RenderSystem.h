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
        const float radius = cshape.radius;
        const size_t points = cshape.points;
        const sf::Color fillColor = cshape.fillColor;

        sf::Vector2f center = {ctransform.position.x, ctransform.position.y};

        for (size_t i = 0; i < points; ++i)
        {
            float angle1 = (i * 2 * M_PI) / points;
            float angle2 = ((i + 1) * 2 * M_PI) / points;

            sf::Vector2 point1 = {center.x + std::cos(angle1) * radius, center.y + std::sin(angle1) * radius};
            sf::Vector2 point2 = {center.x + std::cos(angle2) * radius, center.y + std::sin(angle2) * radius};

            m_shapeBatch.append(sf::Vertex(center, fillColor));
            m_shapeBatch.append(sf::Vertex(point1, fillColor));
            m_shapeBatch.append(sf::Vertex(point2, fillColor));
        }
    }

  public:
    RenderSystem(sf::RenderWindow& window, ArchetypeRegistry& archetypeRegistry)
        : m_window(window),
          m_archetypeRegistry(archetypeRegistry),
          renderShapesQuery(m_archetypeRegistry.CreateQuery<CShape,CTransform>())
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
