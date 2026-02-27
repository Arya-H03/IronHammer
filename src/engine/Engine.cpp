#include "Engine.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "scene/GameScene.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <cstdint>
#include <imgui-SFML.h>
#include <cassert>
#include <memory>

Engine::Engine()
    : m_editorWorld(std::make_unique<World>())
    , m_currentWorld(m_editorWorld.get())
    , m_renderSystem(m_currentWorld)
    , m_inputSystem(m_window)
    , m_editor(m_currentWorld, m_inputSystem, m_renderSystem, m_windowSize, m_gameWindowSize)
{
    Init();
}

void Engine::Init()
{
    m_window.create(sf::VideoMode({ m_windowSize.x, m_windowSize.y }), "IronHammer");

    m_window.setFramerateLimit(m_frameLimit);
    m_window.setKeyRepeatEnabled(false);

    bool ok = ImGui::SFML::Init(m_window);
    assert(ok && "ImGui failed to initialize");

    RegisterScene("GameScene", std::make_unique<GameScene>(m_currentWorld, m_inputSystem, m_gameWindowSize));
    ChangeScene("GameScene");
    m_editor.OnEnter();
}

void Engine::RegisterScene(const std::string& name, std::unique_ptr<BaseScene> scene) { m_scenes[name] = std::move(scene); }

void Engine::ChangeScene(const std::string& name)
{
    auto it = m_scenes.find(name);
    if (it == m_scenes.end()) return;

    if (m_currentScene) m_currentScene->OnExit();

    m_currentScene = it->second.get();
    m_currentScene->OnEnter();
}

void Engine::Update()
{
    float dt = m_clock.restart().asSeconds();
    m_currentWorld->UpdateWorld();
    // ImGui
    ImGui::SFML::Update(m_window, sf::seconds(dt));

    m_inputSystem.PollEvents();

    m_editor.Update(m_renderSystem);
    m_currentScene->Update();

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
