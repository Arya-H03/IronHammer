#pragma once

#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "editor/Viewport.h"
#include <SFML/Graphics/Color.hpp>
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "ecs/component/Components.hpp"
#include "ecs/query/Query.hpp"
#include "ecs/system/ISystem.h"
#include "editor/Viewport.h"
#include <SFML/Graphics/Transform.hpp>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cmath>
#include <cstdint>

class EditorGridSystem : ISetupSystem
{
    struct EditorGridCell
    {
        std::vector<Entity> overlappingEntites;
    };

  private:

    std::vector<EditorGridCell> m_cells;
    sf::Color m_gridColor = sf::Color(255, 255, 255, 40);
    int m_cellSize = 32;
    uint16_t m_cellPerCol;
    uint16_t m_cellPerRow;
    bool m_canShowGrid = true;
    bool m_canSnapToGrid = false;

    Query* m_selectableEntityQuery;

  public:

    EditorGridSystem()
        : m_cellPerCol(std::ceil((float) Viewport::GetSize().y / m_cellSize) + 2)
        , m_cellPerRow(std::ceil((float) Viewport::GetSize().x / m_cellSize) + 2)
    {
    }

    bool& GetCanShowGrid() { return m_canShowGrid; }
    bool& GetCanSnapToGrid() { return m_canSnapToGrid; }
    int& GetCellSize() { return m_cellSize; }
    sf::Color& GetCellColor() { return m_gridColor; }

    void SetupSystem(World* worldPtr) override { m_selectableEntityQuery = worldPtr->Query<RequiredComponents<CTransform, CSprite>>(); }

    void UpdateViewportGrid()
    {
        m_cellPerCol = std::ceil((float) Viewport::GetSize().y / m_cellSize) + 2;
        m_cellPerRow = std::ceil((float) Viewport::GetSize().x / m_cellSize) + 2;

        m_cells.clear();
        m_cells.resize(m_cellPerCol * m_cellPerRow);

        for (const auto& archetype : m_selectableEntityQuery->GetMatchingArchetypes())
        {
            for (const auto& chunk : archetype->GetChunks())
            {
                CTransform* transformCompRow = chunk.GetComponentRow<CTransform>();
                CSprite* spriteCompRow = chunk.GetComponentRow<CSprite>();

                for (size_t i = 0; i < chunk.size; ++i)
                {
                    CTransform& transformComp = transformCompRow[i];
                    CSprite& spriteComp = spriteCompRow[i];

                    Vect2f halfSize = Vect2f(spriteComp.size.x * transformComp.scale.x * 0.5f, spriteComp.size.y * transformComp.scale.y * 0.5f);
                    int minX = (int) std::floor((transformComp.position.x - halfSize.x) / m_cellSize);
                    int maxX = (int) std::floor((transformComp.position.x + halfSize.x) / m_cellSize);
                    int minY = (int) std::floor((transformComp.position.y - halfSize.y) / m_cellSize);
                    int maxY = (int) std::floor((transformComp.position.y + halfSize.y) / m_cellSize);

                    for (int x = minX; x <= maxX; x++)
                    {
                        for (int y = minY; y <= maxY; y++)
                        {
                            if (x < 0 || x >= m_cellPerCol || y < 0 || y >= m_cellPerRow) continue;

                            size_t cellIndex = (x * m_cellPerRow) + y;
                            m_cells[cellIndex].overlappingEntites.push_back(chunk.entities[i]);
                        }
                    }
                }
            }
        }
    }

    Entity GetEntityAtMousePosition(World* worldPtr)
    {
        if (!worldPtr) return Entity {};

        Vect2f mousePos = Viewport::ScreenToViewportMouse();

        int x = (int) std::floor(mousePos.x / m_cellSize);
        int y = (int) std::floor(mousePos.y / m_cellSize);
        if (x < 0 || x >= m_cellPerCol || y < 0 || y >= m_cellPerRow) return Entity {};
        size_t cellIndex = (x * m_cellPerRow) + y;

        Entity closestEntity;
        for (auto entity : m_cells[cellIndex].overlappingEntites)
        {
            CTransform* transform = worldPtr->TryGetComponent<CTransform>(entity);
            CSprite* sprite = worldPtr->TryGetComponent<CSprite>(entity);

            if (!transform || !sprite) return Entity {};

            Vect2f halfSize = Vect2f(sprite->size.x * transform->scale.x * 0.5f, sprite->size.y * transform->scale.y * 0.5f);

            bool hit = mousePos.x >= transform->position.x - halfSize.x && mousePos.x <= transform->position.x + halfSize.x
                       && mousePos.y >= transform->position.y - halfSize.y && mousePos.y <= transform->position.y + halfSize.y;

            if (hit) return entity;
        }
        return Entity {};
    }

    void RenderGrid(sf::RenderTarget& target)
    {
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
