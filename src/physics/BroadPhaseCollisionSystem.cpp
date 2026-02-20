#include "BroadPhaseCollisionSystem.h"
#include "Tracy.hpp"
#include "ecs/component/Components.hpp"
#include "core/utils/Colors.h"

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(
    EntityManager& entityManager, ArchetypeRegistry& archetypeRegistry, Vect2<uint16_t> windowSize)
    : m_entityManger(entityManager)
    , m_windowSize(windowSize)
    , broadPhaseQuery(archetypeRegistry.CreateQuery<RequiredComponents<CCollider, CTransform>>())
{
    m_cellPerCol = m_windowSize.y / m_cellSize;
    m_cellPerRow = m_windowSize.x / m_cellSize;

    PopulateGrid();
}

void BroadPhaseCollisionSystem::PopulateGrid()
{
    m_grid.resize(m_cellPerCol * m_cellPerRow);
    m_entities.resize(m_cellPerCol * m_cellPerRow);

    for (size_t i = 0; i < m_cellPerCol; ++i)
    {
        for (size_t j = 0; j < m_cellPerRow; ++j)
        {
            size_t index = i * m_cellPerRow + j;

            Vect2f pos(((j * m_cellSize) + (m_cellSize / 2)), ((i * m_cellSize) + (m_cellSize / 2)));
            Vect2<int> coord(i, j);
            m_grid[index] = Cell(coord, pos);

            m_entities[index] = m_entityManger.CreateEntity(CTransform(pos, 45, Vect2f(1, 1)),
                CShape(m_cellRadius, 4, Colors::DarkSteel_SFML, sf::Color::White, 2),
                CText(std::to_string(coord.x) + ", " + std::to_string(coord.y),
                    sf::Color::White,
                    Vect2f(m_cellRadius / 3, m_cellRadius / 3),
                    18));
        }
    }
}

void BroadPhaseCollisionSystem::ClearAllCells()
{
    for (size_t i = 0; i < m_grid.size(); ++i)
    {
        m_grid[i].overlapingEntities.clear();
    }
}

void BroadPhaseCollisionSystem::FillCellsWithOverlappingEntities()
{
    for (auto& archetype : broadPhaseQuery.GetMatchingArchetypes())
    {
        for (auto& chunk : archetype->GetChunks())
        {
            auto colliderCompRow = chunk.GetComponentRow<CCollider>();
            auto transformCompRow = chunk.GetComponentRow<CTransform>();

            for (size_t i = 0; i < chunk.size; ++i)
            {
                CCollider& colliderComp = colliderCompRow[i];
                CTransform& transformComp = transformCompRow[i];

                Vect2<int> topLeftCoord =
                    ((transformComp.position + colliderComp.offset - (colliderComp.size / 2)) / m_cellSize).Floor();
                Vect2<int> bottomRightCoord =
                    ((transformComp.position + colliderComp.offset + (colliderComp.size / 2)) / m_cellSize).Ceil();

                for (size_t r = topLeftCoord.y; r < bottomRightCoord.y; ++r)
                {
                    for (size_t c = topLeftCoord.x; c < bottomRightCoord.x; ++c)
                    {
                        if (r < 0 || r >= m_cellPerCol || c < 0 || c >= m_cellPerRow) continue;
                        size_t index = r * m_cellPerRow + c;
                        m_grid[index].overlapingEntities.push_back(chunk.entities[i]);
                    }
                }
            }
        }
    }

    if (m_canHighlightGrid)
    {
        for (size_t i = 0; i < m_grid.size(); ++i)
        {
            size_t count = m_grid[i].overlapingEntities.size();
            CShape& shape = m_entityManger.GetComponentRef<CShape>(m_entities[i]);
            if (count >= 2)
            {
                shape.fillColor = Colors::RustRed_SFML;
            }
            else if (count > 0 && count < 2)
            {
                shape.fillColor = Colors::HazardYellow_SFML;
            }
            else
            {
                shape.fillColor = Colors::DarkSteel_SFML;
            }
        }
    }
}

void BroadPhaseCollisionSystem::FindUniqueCollisionPairs()
{
    m_uniquePairs.clear();

    for (auto& cell : m_grid)
    {
        const size_t count = cell.overlapingEntities.size();
        for (size_t i = 0; i < count; ++i)
        {
            for (size_t j = i + 1; j < count; ++j)
            {
                Entity e1 = cell.overlapingEntities[i];
                Entity e2 = cell.overlapingEntities[j];

                if (e1.id > e2.id) std::swap(e1, e2);
                m_uniquePairs.insert({ e1, e2 });
            }
        }
    }
}

bool BroadPhaseCollisionSystem::GetToggleGrid() const { return m_toggleGrid; }
bool BroadPhaseCollisionSystem::GetCanHighlightGrid() const { return m_canHighlightGrid; }

void BroadPhaseCollisionSystem::SetToggleGrid(bool val) { m_toggleGrid = val; }

void BroadPhaseCollisionSystem::SetCanHighlightGrid(bool val)
{
    m_canHighlightGrid = val;
    if (!m_canHighlightGrid)
    {
        for (size_t i = 0; i < m_grid.size(); ++i)
        {
            m_entityManger.GetComponentRef<CShape>(m_entities[i]).fillColor = Colors::DarkSteel_SFML;
        }
    }
}

void BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem()
{
    ClearAllCells();
    {
        ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");
        FillCellsWithOverlappingEntities();
    }
    {
        ZoneScopedN("BroadPhaseSystem/FindUniqueCollisionPairs");
        FindUniqueCollisionPairs();
    }
}
