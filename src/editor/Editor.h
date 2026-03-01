#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cerrno>
#include <functional>
#include "ecs/World.hpp"
#include "engine/Engine.h"
#include "input/InputManager.h"
#include "input/InputSystem.h"
#include "rendering/RenderSystem.h"
#include "editor/EditorGui.h"

struct EditorFrameContext
{
    RenderSystem& renderSystem;
    InputSystem& inputSystem;
    World* world;
};

class Editor
{
  private:

    Engine& m_engine;
    InputManager m_inputManager;

    sf::RenderWindow& m_window;
    sf::RenderTexture m_renderTexture;

    EditorConfig::Layout m_editorLayout;
    EditorGuiCallbacks m_editorGuiCallbacks;
    EditorGui m_editorGui;

    void RenderViewport(RenderSystem& renderSystem)
    {
        m_renderTexture.clear();
        renderSystem.HandleRenderSystem(m_renderTexture);
        m_renderTexture.display();
    }

  public:

    const sf::RenderTexture& GetViewportRenderTexture() const { return m_renderTexture; }

    Editor(Engine& engine)
        : m_engine(engine)
        , m_window(m_engine.GetRenderWindow())
        , m_editorLayout(m_engine.GetWindowSize())
        , m_editorGui(m_editorLayout, m_editorGuiCallbacks)
        , m_editorGuiCallbacks([&]() { m_engine.TogglePlayMode(); }, [&]() { m_engine.PausePlayMode(); }, [&]() { m_window.close(); })
    {
        m_renderTexture = sf::RenderTexture({ m_editorLayout.Viewport_Width, m_editorLayout.Viewport_Height });
        m_editorGui.ApplyGuiTheme();

        m_inputManager.CreateInputAction("Exit", sf::Keyboard::Key::Escape, InputTrigger::Pressed, [&]() { m_window.close(); });
    }

    void OnEnter() { }
    void OnExit() { }
    void Update(EditorFrameContext& frameContext)
    {
        m_inputManager.Update(frameContext.inputSystem);

        m_editorGui.HandleGUISystem(
            frameContext.renderSystem, frameContext.world, m_renderTexture, m_engine.GetEngineMode(), m_engine.GetIsPlayModePaused());

        RenderViewport(frameContext.renderSystem);
    }
};
