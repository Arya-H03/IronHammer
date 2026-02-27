#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "ecs/World.hpp"
#include "input/InputManager.h"
#include "rendering/RenderSystem.h"
#include "core/utils/Vect2.hpp"
#include "editor/EditorGui.h"

class Editor
{
  private:

    sf::RenderTexture m_renderTexture;
    World* m_worldPtr;

    InputManager m_inputManager;
    EditorGui m_editorGui;

    void InitViewportRenderTexture(Vect2<uint16_t> gameWindowSize) { m_renderTexture = sf::RenderTexture({ gameWindowSize.x, gameWindowSize.y }); }

    void RenderViewport(RenderSystem& renderSystem)
    {
        m_renderTexture.clear();
        renderSystem.HandleRenderSystem(m_renderTexture);
        m_renderTexture.display();
    }

  public:

    const sf::RenderTexture& GetViewportRenderTexture() const { return m_renderTexture; }

    Editor(World* world, InputSystem& inputSystem, RenderSystem& renderSystem, Vect2<uint16_t> windowSize, Vect2<uint16_t> gameWindowSize)
        : m_worldPtr(world), m_inputManager(inputSystem), m_editorGui(m_worldPtr, renderSystem, m_renderTexture, windowSize)
    {
        InitViewportRenderTexture(gameWindowSize);
    }

    void OnEnter()
    {
        m_editorGui.ApplyGuiTheme();

        m_inputManager.CreateInputAction("Log", sf::Keyboard::Key::L, InputTrigger::Pressed, []() { Log_Info("Info") });
        m_inputManager.CreateInputAction("Warning", sf::Keyboard::Key::W, InputTrigger::Pressed, []() { Log_Warning("Warnings"); });
        m_inputManager.CreateInputAction("Error", sf::Keyboard::Key::E, InputTrigger::Pressed, []() { Log_Error("Error"); });
        m_inputManager.CreateInputAction("Play", sf::Keyboard::Key::P, InputTrigger::Pressed, []() { Log_Error("Error"); });
    }
    void OnExit() { }
    void Update(RenderSystem& renderSystem)
    {
        m_inputManager.Update();
        m_editorGui.HandleGUISystem();

        RenderViewport(renderSystem);
    }
};
