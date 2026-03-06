#pragma once
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>
#include "core/utils/Debug.h"
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"
#include "core/utils/FileHelper.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/component/Components.hpp"
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <utility>

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

    bool CreateEntityTemplate(World& world, Entity entity, const EntityStorageLocation& entityLocation, const std::string& name)
    {
        auto it = m_templateMap.find(name);
        if (it != m_templateMap.end())
        {
            Log_Warning("Tried to create an Entity Template with an already existing name: " + name);
            return false;
        }

        Json entityJson = world.SerializeEntity(entityLocation);

        CShape* cshape = world.TryGetComponent<CShape>(entity);
        sf::Texture tex = GenerateEntityTemplateTexture(cshape);

        auto newTemplate = std::make_unique<EntityTemplate>(tex, name, entityJson);
        m_templateMap.emplace(name, std::move(newTemplate));
        JsonUtility::SaveJsonObjectToFile(entityJson, storageFolder + name + ".json");

        return true;
    }

    sf::Texture GenerateEntityTemplateTexture(CShape* cshape)
    {
        if (!cshape)
        {
            // Return empty texture
            sf::RenderTexture tmp({64, 64});
            tmp.clear(sf::Color::White);
            tmp.display();
            return tmp.getTexture();
        }

        // Compute texture size: radius * 2 + outline
        float texSizeF = cshape->radius * 2.f + cshape->outlineThickness * 2.f;
        unsigned int texSize = static_cast<unsigned int>(std::ceil(texSizeF));

        // Create render texture
        sf::RenderTexture renderTex({ texSize, texSize });

        renderTex.clear(sf::Color::Transparent);

        // Create the shape
        sf::CircleShape shape;
        shape.setPointCount(cshape->points);
        cshape->fillColor.a *= 0.7f;
        shape.setFillColor(cshape->fillColor);
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(cshape->outlineThickness);
        shape.setRadius(cshape->radius);

        // Center the shape in the texture
        shape.setOrigin({ cshape->radius + cshape->outlineThickness, cshape->radius + cshape->outlineThickness });
        shape.setPosition({ texSizeF / 2.f, texSizeF / 2.f });

        renderTex.draw(shape);
        renderTex.display();

        return renderTex.getTexture();
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
        pair.mapped()->entityName = newName;
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

                PendingComponent pendingComponent = ComponentRegistry::GetPendingComponentFromEntityJson<CShape>(entityJson);
                CShape* cshape = ComponentRegistry::GetComponentFromPending<CShape>(pendingComponent);
                sf::Texture tex = GenerateEntityTemplateTexture(cshape);

                auto newTemplate = std::make_unique<EntityTemplate>(std::move(tex), filename, entityJson);
                m_templateMap.emplace(filename, std::move(newTemplate));
            }
        }
    }
};
