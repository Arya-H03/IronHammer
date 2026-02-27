#pragma once

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cstdint>
#include <memory>
#include "core/utils/Debug.h"
#include "ecs/World.hpp"
#include "input/InputManager.h"
#include "rendering/RenderSystem.h"
#include "core/utils/Vect2.hpp"
#include "editor/EditorGui.h"
#include "scene/GameScene.h"
#include "scene/SceneManager.h"

class Editor
{
  private:

    sf::RenderTexture m_renderTexture;
    sf::RenderWindow& m_window;
    SceneManager& m_SceneManager;
    World* m_worldPtr;

    InputManager m_inputManager;
    EditorConfig::Layout m_editorLayout;
    EditorGui m_editorGui;

    void RenderViewport(RenderSystem& renderSystem)
    {
        m_renderTexture.clear();
        renderSystem.HandleRenderSystem(m_renderTexture);
        m_renderTexture.display();
    }

  public:

    const sf::RenderTexture& GetViewportRenderTexture() const { return m_renderTexture; }

    Editor(World* world,
        SceneManager& sceneManager,
        sf::RenderWindow& window,
        InputSystem& inputSystem,
        RenderSystem& renderSystem,
        Vect2<uint16_t> windowSize)
        : m_worldPtr(world)
        , m_SceneManager(sceneManager)
        , m_window(window)
        , m_inputManager(inputSystem)
        , m_editorLayout(windowSize)
        , m_editorGui(m_worldPtr, m_editorLayout, renderSystem, m_renderTexture, windowSize)
    {
        m_renderTexture = sf::RenderTexture({ m_editorLayout.Viewport_Width, m_editorLayout.Viewport_Height });
        m_SceneManager.RegisterScene("GameScene",
            std::make_unique<GameScene>(world, inputSystem, Vect2<uint16_t>(m_editorLayout.Viewport_Width , m_editorLayout.Viewport_Height)));
    }

    void OnEnter()
    {
        m_editorGui.ApplyGuiTheme();

        m_inputManager.CreateInputAction("Log", sf::Keyboard::Key::L, InputTrigger::Pressed, []() { Log_Info("Info") });
        m_inputManager.CreateInputAction("Warning", sf::Keyboard::Key::W, InputTrigger::Pressed, []() { Log_Warning("Warnings"); });
        m_inputManager.CreateInputAction("Error", sf::Keyboard::Key::E, InputTrigger::Pressed, []() { Log_Error("Error"); });
        m_inputManager.CreateInputAction("Exit", sf::Keyboard::Key::Escape, InputTrigger::Pressed, [&]() { m_window.close(); });
        m_inputManager.CreateInputAction("Play",
            sf::Keyboard::Key::P,
            InputTrigger::Pressed,
            [&]()
            {
                m_SceneManager.ChangeScene("GameScene");
                Log_Info("Changed to GameScene");
            });
    }
    void OnExit() { }
    void Update(RenderSystem& renderSystem)
    {
        m_inputManager.Update();
        m_editorGui.HandleGUISystem();

        RenderViewport(renderSystem);
    }
};
