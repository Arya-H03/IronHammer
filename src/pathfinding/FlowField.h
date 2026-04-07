#pragma once

#include "core/utils/Vect2.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

using FlowCellCost = int16_t;

enum class FlowCellCostEnum : FlowCellCost
{
    Target     = 0,
    UnWalkable = 100,
    UnVisited  = 500
};
struct FlowCell
{
    Vect2<size_t> gridCoord;
    Vect2f        worldPosition;
    Vect2<int>    flowDir;
    int           baseCost      = 0;
    int           dynamiqueCost = 0;

    void ResetTotalCost()
    {
        baseCost      = 0;
        dynamiqueCost = 0;
    }

    int GetTotalCost() { return baseCost + dynamiqueCost; }
};

static inline const std::vector<Vect2<int>> NeighboorGridDirections = {
    {Vect2<int>{1, 1}, Vect2<int>{1, 0}, Vect2<int>{1, -1}, Vect2<int>{-1, 1}, Vect2<int>{-1, 0}, Vect2<int>{-1, -1},
     Vect2<int>{0, 1}, Vect2<int>{0, -1}}};

class FlowField
{
private:
    std::vector<FlowCell> m_flowCells;

    Vect2f m_originWorldPosition;
    size_t m_gridRows     = 0;
    size_t m_gridCols     = 0;
    size_t m_cellSize     = 1;
    float  m_halfCellSize = 0.5f;

    void CreateFlowGrid()
    {
        m_flowCells.reserve(m_gridCols * m_gridRows);

        for (size_t j = 0; j < m_gridRows; ++j) {
            for (size_t i = 0; i < m_gridCols; ++i) {
                size_t index = j * m_gridCols + i;

                Vect2<size_t> cellCoord = Vect2<size_t>{i, j};
                Vect2f        cellPos   = m_originWorldPosition + Vect2f{(float)i * m_cellSize, (float)j * m_cellSize} -
                                 Vect2f{m_halfCellSize, m_halfCellSize};

                m_flowCells.push_back(
                    FlowCell{cellCoord, cellPos, {0, 0}, static_cast<FlowCellCost>(FlowCellCostEnum::UnVisited), 0});
            }
        }
    }

public:
    FlowField() = default;
    FlowField(Vect2f pos, size_t gridWidth, size_t gridHeight, size_t cellSize)
        : m_originWorldPosition(pos), m_gridRows(std::ceil((float)gridHeight / cellSize)),
          m_gridCols(std::ceil((float)gridWidth / cellSize)), m_cellSize(cellSize), m_halfCellSize((float)cellSize / 2)
    {
        CreateFlowGrid();
    }

    std::vector<FlowCell>& GetCells() { return m_flowCells; }

    size_t GetGridRows() const { return m_gridRows; }
    size_t GetGridCols() const { return m_gridCols; }
    size_t GetCellSize() const { return m_cellSize; }

    // No negative yet
    FlowCell* TryGetFlowCellByWorldPos(const Vect2f& pos)
    {
        int i = std::ceil(pos.x / m_cellSize);
        int j = std::ceil(pos.y / m_cellSize);
        if (i < 0 || i >= m_gridCols || j < 0 || j >= m_gridRows) { return nullptr; }

        size_t index = j * m_gridCols + i;
        return &m_flowCells[index];
    }

    FlowCell* TryGetFlowCellNeighbor(const FlowCell& flowCell, const Vect2<int>& neighborOffset)
    {
        int i = neighborOffset.x + flowCell.gridCoord.x;
        int j = neighborOffset.y + flowCell.gridCoord.y;
        if (i < 0 || i >= m_gridCols || j < 0 || j >= m_gridRows) { return nullptr; }

        size_t index = j * m_gridCols + i;
        return &m_flowCells[index];
    }
};
