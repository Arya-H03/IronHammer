#pragma once
#include <unordered_map>
#include <string>
#include <memory>

#include "scene/BaseScene.h"

class SceneManager
{
  private:

    std::unordered_map<std::string, std::unique_ptr<BaseScene>> m_scenes;
    BaseScene* m_currentScene = nullptr;

  public:

    BaseScene* GetCurrentScenePtr()const {return m_currentScene;}

    void RegisterScene(const std::string& name, std::unique_ptr<BaseScene> scene){ m_scenes[name] = std::move(scene); }

    void ChangeScene(const std::string& name)
    {
        auto it = m_scenes.find(name);
        if (it == m_scenes.end()) return;

        if (m_currentScene) m_currentScene->OnExit();

        m_currentScene = it->second.get();
        m_currentScene->OnEnter();
    }
};
