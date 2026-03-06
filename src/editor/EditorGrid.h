#pragma once

#include "core/utils/Vect2.hpp"
#include "editor/Viewport.h"
#include <SFML/Graphics/Color.hpp>
#include <cmath>
#include <cstdint>
class EditorGrid
{
  private:

    sf::Color m_gridColor = sf::Color(255, 255, 255, 40);
    int m_cellSize = 32;
    uint16_t m_viewPortWidth;
    uint16_t m_viewPortHeight;
    uint16_t m_cellPerCol;
    uint16_t m_cellPerRow;
    bool m_canShowGrid = true;
    bool m_canSnapToGrid = false;

  public:

    EditorGrid() : m_viewPortWidth(Viewport::GetSize().x), m_viewPortHeight(Viewport::GetSize().y) { }

    bool& GetCanShowGrid() { return m_canShowGrid; }
    bool& GetCanSnapToGrid() { return m_canSnapToGrid; }
    int& GetCellSize() { return m_cellSize; }
    sf::Color& GetCellColor() { return m_gridColor; }

    void RenderGrid(sf::RenderTarget& target)
    {
        m_cellPerCol = std::ceil(m_viewPortHeight / m_cellSize) + 2;
        m_cellPerRow = std::ceil(m_viewPortWidth / m_cellSize) + 2;

        if (!m_canShowGrid) return;

        sf::VertexArray grid(sf::PrimitiveType::Lines);

        float width = m_cellPerRow * m_cellSize;
        float height = m_cellPerCol * m_cellSize;

        // Vertical lines
        for (int x = 0; x <= m_cellPerRow; x++)
        {
            float xpos = x * m_cellSize;
            grid.append(sf::Vertex({ xpos, 0.f }, m_gridColor));
            grid.append(sf::Vertex({ xpos, height }, m_gridColor));
        }

        // Horizontal lines
        for (int y = 0; y <= m_cellPerCol; y++)
        {
            float ypos = y * m_cellSize;
            grid.append(sf::Vertex({ 0.f, ypos }, m_gridColor));
            grid.append(sf::Vertex({ width, ypos }, m_gridColor));
        }

        target.draw(grid);
    }
};
