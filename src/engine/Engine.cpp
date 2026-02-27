#include "Engine.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "scene/GameScene.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <imgui-SFML.h>
#include <cassert>
#include <memory>

Engine::Engine()
    : m_editorWorld(std::make_unique<World>())
    , m_currentWorld(m_editorWorld.get())
    , m_renderSystem(m_currentWorld)
    , m_inputSystem(m_window)
    , m_editor(m_currentWorld,m_sceneManager,m_window, m_inputSystem, m_renderSystem, m_windowSize)
{
    Init();
}

void Engine::Init()
{
    m_window.create(sf::VideoMode::getDesktopMode(), "IronHammer",sf::Style::None);

    m_window.setFramerateLimit(m_frameLimit);
    m_window.setKeyRepeatEnabled(false);

    bool ok = ImGui::SFML::Init(m_window);
    assert(ok && "ImGui failed to initialize");

    m_editor.OnEnter();
}

void Engine::Update()
{
    float dt = m_clock.restart().asSeconds();
    m_currentWorld->UpdateWorld();
    // ImGui
    ImGui::SFML::Update(m_window, sf::seconds(dt));

    m_inputSystem.PollEvents();

    m_editor.Update(m_renderSystem);
    if(m_sceneManager.GetCurrentScenePtr())m_sceneManager.GetCurrentScenePtr()->Update();

    // Main Window
    m_window.clear();
    ImGui::SFML::Render(m_window);
    m_window.display();

    m_inputSystem.ClearEvents();
    ++m_currentFrame;
}
void Engine::Run()
{
    while (m_window.isOpen())
    {
        Update();
    }
}
