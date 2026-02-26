#include "Engine.h"
#include "scene/GameScene.h"
#include <imgui-SFML.h>
#include <cassert>
#include <memory>

Engine::Engine() : m_inputSystem(m_window) { Init(); }

void Engine::Init()
{
    m_window.create(sf::VideoMode({ m_windowSize.x, m_windowSize.y }), "IronHammer");

    m_window.setFramerateLimit(m_frameLimit);
    m_window.setKeyRepeatEnabled(false);

    bool ok = ImGui::SFML::Init(m_window);
    assert(ok && "ImGui failed to initialize");

    RegisterScene("GameScene", std::make_unique<GameScene>(m_window,m_inputSystem,m_windowSize));
    ChangeScene("GameScene");
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
    ImGui::SFML::Update(m_window, m_clock.restart());
    m_inputSystem.PollEvents();

    if (m_currentScene) m_currentScene->Update();

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
