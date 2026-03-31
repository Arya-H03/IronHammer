#pragma once

#include "core/utils/Debug.h"
#include "ecs/common/ECSCommon.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "editor/Gizmo.h"
#include "EditorContext.h"
#include "EditorGrid.h"
#include "EditorGui.h"
#include "engine/Engine.h"
#include "input/InputManager.h"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <string>

class Editor
{
    friend class EditorDebugger;

private:
    EditorContext m_editorContext;
    EditorGui     m_gui;
    InputManager  m_inputManager;

    void RenderViewportTexture()
    {
        m_editorContext.viewportTexture.clear();
        m_editorContext.renderSystem->HandleRenderSystem(m_editorContext.viewportTexture);
        if (m_editorContext.engineMode == EngineMode::Edit)
            m_editorContext.editorGrid.RenderGrid(m_editorContext.viewportTexture);
        m_editorContext.viewportTexture.display();
    }

public:
    Editor(Engine& engine) : m_editorContext(engine), m_gui(m_editorContext)
    {
        SystemDebuggerHub::Get().GetEditorDebugger().RegisterEditor(this);

        m_gui.ApplyGuiTheme();

        // One time sync into context
        m_editorContext.renderSystem          = &m_editorContext.engine.GetRenderSystem();
        m_editorContext.inputSystem           = &m_editorContext.engine.GetInputSystem();
        m_editorContext.entityTemplateManager = &m_editorContext.engine.GetEntityTemplateManager();

        Debug::StartLoggerThread();

        m_inputManager.CreateInputAction("Exit", sf::Keyboard::Key::Escape, InputTrigger::Pressed,
                                         [&]() { m_editorContext.engine.GetRenderWindow().close(); });

        m_inputManager.CreateInputAction("Save", sf::Keyboard::Key::S, InputTrigger::Pressed,
                                         [&]() { m_editorContext.engine.SaveEditWorldData(); });

        m_inputManager.CreateInputAction("SelectLiveEntity", sf::Mouse::Button::Left, InputTrigger::Pressed, [&]() {
            Entity entity = m_editorContext.editorGrid.GetEntityAtMousePosition(m_editorContext.world);
            if (entity.id != InvalidEntityID)
                m_editorContext.inspector.InspectLiveEntity(entity, m_editorContext.world->GetEntityManager());
        });

        m_inputManager.CreateInputAction("GizmoPositionMode", sf::Keyboard::Key::W, InputTrigger::Pressed,
                                         [&]() { m_editorContext.viewPortGizmMode = GismoMode::Position; });
        m_inputManager.CreateInputAction("GizmoScaleMode", sf::Keyboard::Key::E, InputTrigger::Pressed,
                                         [&]() { m_editorContext.viewPortGizmMode = GismoMode::Scale; });
    }

    ~Editor() { SystemDebuggerHub::Get().GetEditorDebugger().UnRegisterEditor(); }

    void Update()
    {
        // Sync runtime into context
        m_editorContext.world            = m_editorContext.engine.GetCurrentWorld();
        m_editorContext.engineMode       = m_editorContext.engine.GetEngineMode();
        m_editorContext.isPlayModePaused = m_editorContext.engine.GetIsPlayModePaused();
        m_editorContext.editorGrid.SetupSystem(m_editorContext.world);
        m_editorContext.editorGrid.UpdateViewportGrid();

        m_inputManager.Update(*m_editorContext.inputSystem);

        RenderViewportTexture();

        m_gui.Update();
    }
};
