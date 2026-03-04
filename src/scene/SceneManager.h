#pragma once
#include <fstream>
#include <unordered_map>
#include <string>
#include <memory>

#include "ecs/World.hpp"
#include "scene/BaseScene.h"
#include "core/saving/JsonUtility.h"

class SceneManager
{
  private:

    std::unordered_map<std::string, std::unique_ptr<BaseScene>> m_scenes;
    BaseScene* m_currentScene = nullptr;

  public:

    BaseScene* GetCurrentScenePtr() const { return m_currentScene; }

    void RegisterScene(const std::string& name, std::unique_ptr<BaseScene> scene)
    {
        m_scenes[name] = std::move(scene);
        Log_Info(name + " Scene was registered");
    }

    void ChangeScene(const std::string& name, World* worldPtr)
    {
        auto it = m_scenes.find(name);
        // Return if scene not found or already playing
        if (it == m_scenes.end()) return;

        if (m_currentScene) m_currentScene->OnChangeFrom(worldPtr);

        m_currentScene = it->second.get();
        m_currentScene->OnChangeTo(worldPtr);

        Log_Info("Changed to Scene " + name);
    }

    void SaveScene(World& world, const std::string& filePath)
    {
        Json sceneJson = world.SerializeWorld();
        JsonUtility::SaveJsonObjectToFile(sceneJson, filePath);
    }

    void LoadScene(World& world, const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            Log_Warning("Failed to open scene file: " + filePath);
            return;
        }

        Json sceneJson = JsonUtility::LoadJsonObjectFromFile(filePath);
        world.DeserializeWorld(sceneJson);
    }
};
