#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include "editor/EditorGrid.h"
#include "engine/Engine.h"
#include "ecs/World.hpp"
#include "ecs/entity/EntityTemplateManager.h"
#include "ecs/entity/EntityInspector.h"
#include "ecs/archetype/ArchetypeDebugger.hpp"
#include "editor/EditorConfig.h"

struct EditorContext
{
    Engine& engine;

    // Synced every frame
    World* world = nullptr;
    EngineMode engineMode = EngineMode::Edit;
    bool isPlayModePaused = false;

    // Sync once
    RenderSystem* renderSystem = nullptr;
    InputSystem* inputSystem = nullptr;
    EntityTemplateManager* entityTemplateManager = nullptr;

    EntityInspector inspector;
    ArchetypeDebugger archetypeDebugger;
    EditorConfig::Layout layout;
    EditorGrid editorGrid;

    sf::RenderTexture viewportTexture;


    EditorContext(Engine& engine) : engine(engine), layout(engine.GetWindowSize())
    {
        viewportTexture = sf::RenderTexture({ layout.Viewport_Width, layout.Viewport_Height });
    }
};
