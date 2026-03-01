#include "Engine.h"

#include "core/utils/Debug.h"
#include "core/utils/Vect2.hpp"
#include "ecs/World.hpp"
#include "input/InputSystem.h"
#include "rendering/RenderSystem.h"
#include "scene/BaseScene.h"
#include "scene/GameScene.h"
#include "scene/SceneManager.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <cstdint>
#include <imgui-SFML.h>
#include <cassert>
#include <memory>

Engine::Engine()
    : m_editorWorld(std::make_unique<World>()), m_currentWorld(m_editorWorld.get()), m_renderSystem(m_currentWorld), m_inputSystem(m_window)
{
    Init();
}

bool Engine::GetIsPlayModePaused() const
{
    if (!m_sceneManager.GetCurrentScenePtr())
    {
        Log_Warning("Tried to GetIsPlayModePaused when there is not currentScene");
        return false;
    }

    return m_sceneManager.GetCurrentScenePtr()->GetScenePaused();
}
EngineMode Engine::GetEngineMode() const { return m_engineMode; }
Vect2<uint16_t> Engine::GetWindowSize() const { return m_windowSize; }
World* Engine::GetCurrentWorld() const { return m_currentWorld; }
sf::RenderWindow& Engine::GetRenderWindow() { return m_window; }
RenderSystem& Engine::GetRenderSystem() { return m_renderSystem; }
SceneManager& Engine::GetSceneManager() { return m_sceneManager; }
InputSystem& Engine::GetInputSystem() { return m_inputSystem; }

void Engine::Init()
{
    m_window.create(sf::VideoMode::getDesktopMode(), "IronHammer", sf::Style::None);
    m_window.setFramerateLimit(m_frameLimit);
    m_window.setKeyRepeatEnabled(false);

    bool ok = ImGui::SFML::Init(m_window);
    assert(ok && "ImGui failed to initialize");

    m_sceneManager.RegisterScene("Game", std::make_unique<GameScene>(m_windowSize));
    m_sceneManager.ChangeScene("Game", m_currentWorld);
}

void Engine::PausePlayMode()
{
    if (!m_sceneManager.GetCurrentScenePtr()) return;
    bool val = m_sceneManager.GetCurrentScenePtr()->GetScenePaused() ? false : true;
    m_sceneManager.GetCurrentScenePtr()->SetScenePaused(val);
}

void Engine::EnterPlayMode()
{
    m_tempWorld.reset();
    m_tempWorld = std::make_unique<World>();
    m_currentWorld = m_tempWorld.get();

    m_renderSystem.ResetWorld(m_currentWorld);
    m_sceneManager.GetCurrentScenePtr()->OnStartPlay(m_currentWorld);

    m_engineMode = EngineMode::Play;
    Log_Info("Changed to PlayMode");
}

void Engine::ExitPlayMode()
{
    m_currentWorld = m_editorWorld.get();

    m_renderSystem.ResetWorld(m_currentWorld);
    m_sceneManager.GetCurrentScenePtr()->OnExitPlay(m_currentWorld);

    m_engineMode = EngineMode::Edit;
    Log_Info("Changed to EditMode");
}

void Engine::TogglePlayMode()
{
    if (m_engineMode == EngineMode::Edit)
    {
        EnterPlayMode();
    }
    else if (m_engineMode == EngineMode::Play)
    {
        ExitPlayMode();
    }
}

float Engine::BeginFrame()
{
    float dt = m_clock.restart().asSeconds();

    ImGui::SFML::Update(m_window, sf::seconds(dt));
    m_inputSystem.PollEvents();

    return dt;
}

void Engine::UpdateRuntime()
{
    m_currentWorld->UpdateWorld();

    if (m_sceneManager.GetCurrentScenePtr() && m_engineMode == EngineMode::Play)
        m_sceneManager.GetCurrentScenePtr()->Update(m_currentWorld, m_inputSystem);
}

void Engine::RenderFrame()
{
    m_window.clear();
    ImGui::SFML::Render(m_window);
    m_window.display();
}

void Engine::EndFrame()
{
    m_inputSystem.ClearEvents();
    ++m_currentFrame;
}
