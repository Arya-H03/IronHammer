#include "BroadPhaseCollisionSystem.h"

#include "core/utils/Colors.h"
#include "ecs/component/Components.hpp"
#include "Tracy.hpp"

#include <SFML/Graphics/Color.hpp>
#include <unordered_set>

BroadPhaseCollisionSystem::BroadPhaseCollisionSystem(Vect2<uint16_t> windowSize) : m_windowSize(windowSize)
{
    m_cellPerCol = m_windowSize.y / m_cellSize;
    m_cellPerRow = m_windowSize.x / m_cellSize;
}

void BroadPhaseCollisionSystem::SetupSystem(World* worldPtr)
{
    m_broadPhaseQuery = worldPtr->Query<RequiredComponents<CTransform, CCollider>>();
    PopulateGrid(worldPtr);

    // SetCanDisplayGrid(worldPtr, true);
    // SetCanHighlightGrid(worldPtr, true);
}

void BroadPhaseCollisionSystem::PopulateGrid(World* worldPtr)
{
    m_grid.resize(m_cellPerCol * m_cellPerRow);
    m_gridDisplayEntities.resize(m_cellPerCol * m_cellPerRow);

    for (size_t i = 0; i < m_cellPerCol; ++i) {
        for (size_t j = 0; j < m_cellPerRow; ++j) {
            size_t index = i * m_cellPerRow + j;

            Vect2f     pos(((j * m_cellSize) + (m_cellSize * 0.5f)), ((i * m_cellSize) + (m_cellSize * 0.5f)));
            Vect2<int> coord(i, j);
            m_grid[index] = Cell(coord, pos);

            m_gridDisplayEntities[index] = worldPtr->CreateEntityWithReturn(
                CTransform(pos, Vect2f(1, 1), 0),
                CShape(4, Colors::DarkSteel_SFML, sf::Color(255, 255, 255, 100), m_cellRadius, 2),
                CText(std::to_string(coord.x) + ", " + std::to_string(coord.y), sf::Color::White,
                      Vect2f(m_cellRadius / 3, m_cellRadius / 3), 18),
                CNotDrawable{});
        }
    }
}

void BroadPhaseCollisionSystem::ClearAllCells()
{
    for (size_t i = 0; i < m_grid.size(); ++i) { m_grid[i].overlapingEntities.clear(); }
}

void BroadPhaseCollisionSystem::FillCellsWithOverlappingEntities(World* worldPtr)
{
    m_broadPhaseQuery->ForEachWithEntity<CCollider, CTransform>(
        [&](Entity entity, CCollider& colliderComp,CTransform& transformComp) {
            Vect2<int> topLeftCoord =
                ((transformComp.position + colliderComp.offset - (colliderComp.halfSize)) / m_cellSize).Floor();
            Vect2<int> bottomRightCoord =
                ((transformComp.position + colliderComp.offset + (colliderComp.halfSize)) / m_cellSize).Ceil();

            for (size_t r = topLeftCoord.y; r < bottomRightCoord.y; ++r) {
                for (size_t c = topLeftCoord.x; c < bottomRightCoord.x; ++c) {
                    if (r < 0 || r >= m_cellPerCol || c < 0 || c >= m_cellPerRow) continue;
                    size_t index = r * m_cellPerRow + c;
                    m_grid[index].overlapingEntities.push_back(entity);
                }
            }
        });

    if (m_canHighlightGrid) {
        for (size_t i = 0; i < m_grid.size(); ++i) {
            size_t  count = m_grid[i].overlapingEntities.size();
            CShape* shape = worldPtr->TryGetComponent<CShape>(m_gridDisplayEntities[i]);
            if (!shape) continue;
            if (count >= 2) { shape->fillColor = Colors::RustRed_SFML; }
            else if (count == 1) {
                shape->fillColor = Colors::HazardYellow_SFML;
            }
            else {
                shape->fillColor = Colors::DarkSteel_SFML;
            }
        }
    }
}

void BroadPhaseCollisionSystem::FindUniqueCollisionPairs()
{
    m_uniquePotentialPairsSet.clear();
    for (auto& cell : m_grid) {
        const size_t count = cell.overlapingEntities.size();
        for (size_t i = 0; i < count; ++i) {
            for (size_t j = i + 1; j < count; ++j) {
                Entity e1 = cell.overlapingEntities[i];
                Entity e2 = cell.overlapingEntities[j];
                m_uniquePotentialPairsSet.insert(CollisionPair(e1, e2));
            }
        }
    }

    m_uniquePotentialPairsVector.clear();
    m_uniquePotentialPairsVector.reserve(m_uniquePotentialPairsVector.size());
    for (auto& pair : m_uniquePotentialPairsSet) { m_uniquePotentialPairsVector.push_back(pair); }
}

bool BroadPhaseCollisionSystem::GetCanDisplayGrid() const { return m_canDisplayGrid; }
bool BroadPhaseCollisionSystem::GetCanHighlightGrid() const { return m_canHighlightGrid; }

void BroadPhaseCollisionSystem::SetCanDisplayGrid(World* worldPtr, bool val)
{
    m_canDisplayGrid = val;

    if (val == true) {
        for (auto& entity : m_gridDisplayEntities) { worldPtr->RemoveFromEntity<CNotDrawable>(entity); }
        SetCanHighlightGrid(worldPtr, true);
    }
    else {
        for (auto& entity : m_gridDisplayEntities) { worldPtr->AddToEntity(entity, CNotDrawable{}); }

        SetCanHighlightGrid(worldPtr, false);
    }
}

void BroadPhaseCollisionSystem::SetCanHighlightGrid(World* worldPtr, bool val)
{
    m_canHighlightGrid = val;
    if (!m_canHighlightGrid) {
        for (size_t i = 0; i < m_grid.size(); ++i) {
            worldPtr->TryGetComponent<CShape>(m_gridDisplayEntities[i])->fillColor = Colors::DarkSteel_SFML;
        }
    }
}

std::vector<CollisionPair>& BroadPhaseCollisionSystem::HandleBroadPhaseCollisionSystem(World* worldPtr)
{
    ClearAllCells();
    {
        ZoneScopedN("BroadPhaseSystem/FillCellsWithOverlappingEntities");
        FillCellsWithOverlappingEntities(worldPtr);
    }
    {
        ZoneScopedN("BroadPhaseSystem/FindUniqueCollisionPairs");
        FindUniqueCollisionPairs();
    }

    return m_uniquePotentialPairsVector;
}
