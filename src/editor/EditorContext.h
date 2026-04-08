#pragma once

#include "ecs/World.hpp"
#include "ecs/entity/EntityTemplateManager.h"
#include "editor/EditorConfig.h"
#include "editor/EditorGrid.h"
#include "editor/Gizmo.h"
#include "editor/Viewport.h"
#include "editor/entityInspector/EntityInspector.h"
#include "engine/Engine.h"

#include <SFML/Graphics/RenderTexture.hpp>

struct EditorContext
{
    Engine& engine;

    // Synced every frame
    World* world = nullptr;
    EngineMode engineMode = EngineMode::Edit;
    bool isPlayModePaused = false;

    // Sync once
    RenderingSystem* renderSystem = nullptr;
    InputSystem* inputSystem = nullptr;
    EntityTemplateManager* entityTemplateManager = nullptr;

    EntityInspector inspector;
    EditorConfig::Layout layout;
    EditorGridSystem editorGrid;

    sf::RenderTexture viewportTexture;

    GismoMode viewPortGizmMode = GismoMode::Position;

    EditorContext(Engine& engine) : engine(engine), layout(engine.GetWindowSize())
    {
        viewportTexture = sf::RenderTexture({layout.Viewport_Width, layout.Viewport_Height});
        Viewport::SetViewportTexture(&viewportTexture);
    }
};
