#pragma once

#include "EditorContext.h"
#include "EditorGui.h"
#include "EditorGrid.h"
#include "core/utils/Debug.h"
#include "input/InputManager.h"
#include <SFML/Window/Keyboard.hpp>

class Editor
{
  private:

    EditorContext m_editorContext;
    EditorGui m_gui;
    InputManager m_inputManager;

    void RenderViewportTexture()
    {
        m_editorContext.viewportTexture.clear();
        m_editorContext.renderSystem->HandleRenderSystem(m_editorContext.viewportTexture);
        m_editorContext.editorGrid.RenderGrid(m_editorContext.viewportTexture);
        m_editorContext.viewportTexture.display();
    }

  public:

    Editor(Engine& engine) : m_editorContext(engine), m_gui(m_editorContext)
    {
        m_gui.ApplyGuiTheme();

        // One time sync into context
        m_editorContext.renderSystem = &m_editorContext.engine.GetRenderSystem();
        m_editorContext.inputSystem = &m_editorContext.engine.GetInputSystem();
        m_editorContext.entityTemplateManager = &m_editorContext.engine.GetEntityTemplateManager();

        Debug::StartLoggerThread();

        m_inputManager.CreateInputAction(
            "Exit", sf::Keyboard::Key::Escape, InputTrigger::Pressed, [&]() { m_editorContext.engine.GetRenderWindow().close(); });

        m_inputManager.CreateInputAction("Save", sf::Keyboard::Key::S, InputTrigger::Pressed, [&]() { m_editorContext.engine.SaveEditWorldData(); });
        m_inputManager.CreateInputAction("Info", sf::Keyboard::Key::I, InputTrigger::Pressed, [&]() { LOG_INFO("Info"); });
        m_inputManager.CreateInputAction("Warning", sf::Keyboard::Key::W, InputTrigger::Pressed, [&]() {LOG_WARNING("Warning"); });
        m_inputManager.CreateInputAction("Error", sf::Keyboard::Key::E, InputTrigger::Pressed, [&]() { LOG_ERROR("Error"); });
    }

    void Update()
    {
        // Sync runtime into context
        m_editorContext.world = m_editorContext.engine.GetCurrentWorld();
        m_editorContext.engineMode = m_editorContext.engine.GetEngineMode();
        m_editorContext.isPlayModePaused = m_editorContext.engine.GetIsPlayModePaused();

        m_inputManager.Update(*m_editorContext.inputSystem);

        RenderViewportTexture();

        m_gui.Update();
    }
};
