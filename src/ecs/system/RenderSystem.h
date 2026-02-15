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
        MakeSignatureMask<CShape>();
    }

    void HandleRenderSystem()
    {
        m_window.clear();

        for(auto& archetype : m_matchingArchetypes)
        {
            for(auto& chunk: archetype->GetChunks())
            {
                auto renderComps = chunk.GetComponentRow<CShape>();
                for(size_t i = 0; i < chunk.size; ++i)
                {
                    m_window.draw(renderComps[i].shape);
                }
            }
        }

        ImGui::SFML::Render(m_window);
        m_window.display();
    }
};
