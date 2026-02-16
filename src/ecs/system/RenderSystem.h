#pragma once
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Window.hpp>
#include "ecs/component/Components.hpp"
#include "ecs/system/BaseSystem.h"

class RenderSystem: public BaseSystem
{
    private:
   	sf::RenderWindow& m_window;

    public:
    RenderSystem(sf::RenderWindow& window): m_window(window)
    {
        MakeSignatureMask<CShape,CTransform>();
    }

    void HandleRenderSystem()
    {
        m_window.clear();

        for (auto& archetype : m_matchingArchetypes)
        {
            for (auto& chunk : archetype->GetChunks())
            {
                auto shapeCompRow = chunk.GetComponentRow<CShape>();
                auto transformCompRow = chunk.GetComponentRow<CTransform>();

                for (size_t i = 0; i < chunk.size; ++i)
                {
                    const CShape& shapeComp = shapeCompRow[i];
                    const CTransform& transformComp = transformCompRow[i];

                    sf::CircleShape shape(shapeComp.radius, shapeComp.points);
                    shape.setFillColor(shapeComp.fillColor);
                    shape.setOutlineColor(shapeComp.outlineColor);
                    shape.setOutlineThickness(shapeComp.outlineThickness);

                    shape.setPosition({transformComp.position.x, transformComp.position.y});
                    m_window.draw(shape);
                }
            }
        }

        ImGui::SFML::Render(m_window);
        m_window.display();
    }

};
