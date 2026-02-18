#pragma once
#include "core/utils/Random.hpp"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/Components.hpp"
#include "ecs/entity/EntityManager.hpp"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Angle.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>

struct Cell
{
    Vect2<int> coord;
    Vect2f pos;
};

class BroadPhaseCollisionSystem
{
  private:
    EntityManager& m_entityManger;
    Vect2<uint16_t> m_windowSize;
    std::vector<Cell> m_grid;
    std::vector<Entity> m_entities;
    const float m_cellSize = 64;
    const float m_cellRadius= std::sqrt((m_cellSize * m_cellSize) / 2);
    uint16_t m_cellPerRow;
    uint16_t m_cellPerCol;

    void PopulateGrid()
    {
        m_grid.resize(m_cellPerCol * m_cellPerRow);
        m_entities.resize(m_cellPerCol * m_cellPerRow);

        for (size_t i = 0; i < m_cellPerCol; ++i)
        {
            for (size_t j = 0; j < m_cellPerRow; ++j)
            {
                size_t index = i * m_cellPerRow + j;

                Vect2<int> coord(i, j);
                Vect2f pos(((i * m_cellSize) + (m_cellSize / 2)), ((j * m_cellSize) + (m_cellSize / 2)));
                m_grid[index] = Cell(coord, pos);

                m_entities[index] = m_entityManger.CreateEntity(
                    CTransform(pos, 45, Vect2f(1, 1)),
                    CText(std::to_string(coord.x) + ", " + std::to_string(coord.y), sf::Color::White),
                    CShape(m_cellRadius, 4, sf::Color(1,1,1,200), sf::Color::White, 2));
            }
        }
    }

  public:
    BroadPhaseCollisionSystem(EntityManager& entityManager, Vect2<uint16_t> windowSize)
        : m_entityManger(entityManager), m_windowSize(windowSize)
    {
        m_cellPerCol = m_windowSize.x / m_cellSize;
        m_cellPerRow = m_windowSize.y / m_cellSize;

        std::cerr<<m_cellPerRow <<","<<m_cellPerCol;
        PopulateGrid();
    }
};
