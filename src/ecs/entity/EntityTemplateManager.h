#pragma once
#include <string>
#include "core/utils/Debug.h"
#include "ecs/World.hpp"
#include "core/saving/JsonUtility.h"
#include "ecs/common/ECSCommon.h"
#include "core/utils/FileHelper.h"
#include <memory>
#include <unordered_map>
#include <filesystem>

struct EntityTemplate
{
    std::string name;
    Json entityJson;

    EntityTemplate(std::string n, Json j) : name(std::move(n)), entityJson(std::move(j)) { }
};

class EntityTemplateManager
{
  private:

    const std::string storageFolder = "src/assets/entityTemplates/";
    std::unordered_map<std::string, std::unique_ptr<EntityTemplate>> m_templateMap;

  public:

    EntityTemplateManager() { LoadAllEntityTemplates(); }

    const std::unordered_map<std::string, std::unique_ptr<EntityTemplate>>& GetEntityTemplates() const { return m_templateMap; }

    EntityTemplate* GetTemplateByName(const std::string& name)
    {
        auto it = m_templateMap.find(name);
        if (it == m_templateMap.end()) return nullptr;
        return it->second.get();
    }

    bool CreateEntityTemplate(World& world, EntityStorageLocation entityLocation, const std::string& name)
    {
        auto it = m_templateMap.find(name);
        if (it != m_templateMap.end())
        {
            Log_Warning("Tried to create an Entity Template with an already existing name: " + name);
            return false;
        }

        Json entityJson = world.SerializeEntity(entityLocation);
        auto newTemplate = std::make_unique<EntityTemplate>(name, entityJson);
        m_templateMap.emplace(name, std::move(newTemplate));
        JsonUtility::SaveJsonObjectToFile(entityJson, storageFolder + name + ".json");

        return true;
    }

    void UpdateEntityTemplate(Json& entityJson, const std::string& name)
    {
        auto* templatePtr = GetTemplateByName(name);
        if (!templatePtr)
        {
            Log_Warning("Tried to update non-existent template: " + name);
            return;
        }

        templatePtr->entityJson = entityJson;
        JsonUtility::SaveJsonObjectToFile(entityJson, storageFolder + name + ".json");
    }

    void DeleteEntityTemplate(const std::string& name)
    {
        auto it = m_templateMap.find(name);
        if (it == m_templateMap.end())
        {
            Log_Warning("Tried to delete non-existent template: " + name);
            return;
        }

        FileHelper::DeleteFile(storageFolder + name + ".json");
        m_templateMap.erase(it);
    }

    void ChangeEntityTemplateName(const std::string& oldName, const std::string& newName)
    {
        if (m_templateMap.contains(newName))
        {
            Log_Warning("Template already exists: " + newName);
            return;
        }

        auto pair = m_templateMap.extract(oldName);
        if (!pair)
        {
            Log_Warning("Template not found: " + oldName);
            return;
        }

        FileHelper::RenameFile(oldName, storageFolder + oldName + ".json", newName, storageFolder + newName + ".json");

        pair.key() = newName;
        pair.mapped()->name = newName;
        m_templateMap.insert(std::move(pair));
    }

    void LoadAllEntityTemplates()
    {
        for (const auto& entry : std::filesystem::directory_iterator(storageFolder))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                std::string filePath = entry.path().string();
                std::string filename = entry.path().stem().string();

                Json entityJson = JsonUtility::LoadJsonObjectFromFile(filePath);
                auto newTemplate = std::make_unique<EntityTemplate>(filename, entityJson);
                m_templateMap.emplace(filename, std::move(newTemplate));
            }
        }
    }
};
