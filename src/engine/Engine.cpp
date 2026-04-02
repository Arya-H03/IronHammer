#include "Engine.h"

#include "core/utils/Debug.h"
#include "core/utils/Time.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/World.hpp"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "input/InputSystem.h"
#include "rendering/RenderSystem.h"
#include "scene/BaseScene.h"
#include "scene/GameScene.h"
#include "scene/SceneManager.h"

#include <cassert>
#include <cstdint>
#include <imgui-SFML.h>
#include <memory>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/WindowEnums.hpp>

Engine::Engine()
    : m_editorWorld(std::make_unique<World>()), m_currentWorld(m_editorWorld.get()), m_renderSystem(m_currentWorld),
      m_inputSystem(m_window)
{
    Init();
}

bool Engine::GetIsPlayModePaused() const
{
    if (!m_sceneManager.GetCurrentScenePtr()) {
        LOG_WARNING("Tried to GetIsPlayModePaused when there is not currentScene");
        return false;
    }

    return m_sceneManager.GetCurrentScenePtr()->GetScenePaused();
}

EngineMode             Engine::GetEngineMode() const { return m_engineMode; }
Vect2<uint16_t>        Engine::GetWindowSize() const { return m_windowSize; }
World*                 Engine::GetCurrentWorld() const { return m_currentWorld; }
sf::RenderWindow&      Engine::GetRenderWindow() { return m_window; }
RenderingSystem&       Engine::GetRenderSystem() { return m_renderSystem; }
SceneManager&          Engine::GetSceneManager() { return m_sceneManager; }
InputSystem&           Engine::GetInputSystem() { return m_inputSystem; }
EntityTemplateManager& Engine::GetEntityTemplateManager() { return m_entityTemplateManager; }

void Engine::Init()
{
    m_window.create(sf::VideoMode::getDesktopMode(), "IronHammer", sf::Style::None);
    m_window.setKeyRepeatEnabled(false);

    bool ok = ImGui::SFML::Init(m_window);
    assert(ok && "ImGui failed to initialize");

    ComponentRegistry::RegisterAllComponents();

    m_entityTemplateManager.LoadAllEntityTemplates();
    LoadEditorSceneData();

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
    LoadTempSceneData();

    m_currentWorld = m_tempWorld.get();
    SystemDebuggerHub::Instance().GetWorldDebugger().RegisterWorld(m_currentWorld);

    m_renderSystem.SetupSystem(m_currentWorld);
    m_sceneManager.GetCurrentScenePtr()->OnStartPlay(m_currentWorld);

    m_engineMode = EngineMode::Play;
    LOG_INFO("Changed to PlayMode");
}

void Engine::ExitPlayMode()
{
    m_currentWorld = m_editorWorld.get();
    SystemDebuggerHub::Instance().GetWorldDebugger().RegisterWorld(m_currentWorld);

    m_renderSystem.SetupSystem(m_currentWorld);
    m_sceneManager.GetCurrentScenePtr()->OnExitPlay(m_currentWorld);

    m_engineMode = EngineMode::Edit;
    LOG_INFO("Changed to EditMode");
}

void Engine::TogglePlayMode()
{
    if (m_engineMode == EngineMode::Edit) { EnterPlayMode(); }
    else if (m_engineMode == EngineMode::Play) {
        ExitPlayMode();
    }
}

void Engine::SaveEditWorldData()
{
    if (m_engineMode == EngineMode::Play) return;
    m_sceneManager.SaveScene(*m_editorWorld, "src/assets/sceneData/test.Json");
}
void Engine::LoadEditorSceneData()
{
    if (m_engineMode == EngineMode::Play) return;
    m_sceneManager.LoadScene(*m_editorWorld, "src/assets/sceneData/test.Json");
}

void Engine::LoadTempSceneData() { m_sceneManager.LoadScene(*m_tempWorld, "src/assets/sceneData/test.Json"); }

void Engine::BeginFrame()
{
    m_frameRateHandler.OnFrameBegin();
    ImGui::SFML::Update(m_window, sf::seconds(m_clock.restart().asSeconds()));
    m_inputSystem.PollEvents();
}

void Engine::UpdateRuntime()
{
    m_currentWorld->UpdateWorld();

    if (m_sceneManager.GetCurrentScenePtr() && m_engineMode == EngineMode::Play)
        m_sceneManager.GetCurrentScenePtr()->Update(m_currentFrame, m_currentWorld, m_inputSystem);
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
    m_frameRateHandler.OnFrameEnd();
}
