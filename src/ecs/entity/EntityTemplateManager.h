#pragma once
#include "assets/AssetManager.h"
#include "core/CoreComponents.hpp"
#include "core/saving/JsonUtility.h"
#include "core/utils/Debug.h"
#include "core/utils/FileHelper.h"
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "rendering/RenderingComponents.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Angle.hpp>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

class EntityTemplateManager
{
  private:
    const std::string storageFolder = "src/assets/entityTemplates/";
    std::unordered_map<std::string, std::unique_ptr<EntityTemplate>> m_templateMap;
    const uint16_t m_iconTextureSize = 64;
    const float m_iconTexturePadding = 0.8f;

  public:
    EntityTemplateManager() = default;

    const std::unordered_map<std::string, std::unique_ptr<EntityTemplate>>& GetEntityTemplates() const { return m_templateMap; }

    EntityTemplate* GetTemplateByName(const std::string& name)
    {
        auto it = m_templateMap.find(name);
        if (it == m_templateMap.end()) return nullptr;
        return it->second.get();
    }

    bool CreateEntityTemplate(World& world, Entity entity, const EntityStorageLocation& entityLocation, const std::string& name)
    {
        if (m_templateMap.contains(name))
        {
            LOG_WARNING("Tried to create an Entity Template with an already existing name: " + name);
            return false;
        }

        Json entityJson = world.SerializeEntity(entityLocation);

        const sf::Texture texture = GenerateEntityTemplateIconTexture(world, entity);
        m_templateMap.emplace(name, std::make_unique<EntityTemplate>(std::move(texture), name, entityJson));

        JsonUtility::SaveJsonObjectToFile(entityJson, storageFolder + name + ".json");

        return true;
    }

    const sf::Texture GenerateEntityTemplateIconTexture(World& world, Entity entity)
    {
        CSprite* csprite = world.TryGetComponent<CSprite>(entity);
        CTransform* ctransform = world.TryGetComponent<CTransform>(entity);

        sf::RenderTexture renderTexture({m_iconTextureSize, m_iconTextureSize});
        renderTexture.clear(sf::Color(40, 40, 40));

        if (!csprite || !ctransform)
        {
            renderTexture.display();
            return renderTexture.getTexture();
        }

        sf::Texture* texture = AssetManager::Instance().LoadTexture(csprite->textureName);
        if (!texture)
        {
            LOG_ERROR("Could not load texture: " + csprite->textureName);
            renderTexture.display();
            return renderTexture.getTexture();
        }

        sf::Sprite sprite(*texture);
        sprite.setTextureRect(csprite->textureRect);
        sprite.setColor(csprite->color);
        sprite.setOrigin({csprite->textureRect.size.x * 0.5f, csprite->textureRect.size.y * 0.5f});
        sprite.setPosition({m_iconTextureSize * 0.5f, m_iconTextureSize * 0.5f});
        sprite.setRotation(sf::degrees(-ctransform->rotation)); // Negate rotation to match rendering

        float spriteWidth = csprite->size.x * ctransform->scale.x;
        float spriteHeight = csprite->size.y * ctransform->scale.y;
        float scaleX = spriteWidth / (m_iconTexturePadding * m_iconTextureSize);
        float scaleY = spriteHeight / (m_iconTexturePadding * m_iconTextureSize);
        float fitScale = std::max(scaleX, scaleY); // fit to icon size based on longest axis
        sprite.setScale({spriteWidth / (csprite->textureRect.size.x * fitScale), spriteHeight / (csprite->textureRect.size.y * fitScale)});

        renderTexture.draw(sprite);
        renderTexture.display();

        return renderTexture.getTexture();
    }

    const sf::Texture GenerateEntityTemplateIconTexture(Json entityJson) const
    {
        PendingComponent pendingCSprite = ComponentRegistry::GetPendingComponentFromEntityJson<CSprite>(entityJson);
        CSprite* csprite = ComponentRegistry::GetComponentFromPending<CSprite>(pendingCSprite);

        PendingComponent pendingCTransform = ComponentRegistry::GetPendingComponentFromEntityJson<CTransform>(entityJson);
        CTransform* ctransform = ComponentRegistry::GetComponentFromPending<CTransform>(pendingCTransform);

        sf::RenderTexture renderTexture({m_iconTextureSize, m_iconTextureSize});
        renderTexture.clear(sf::Color(40, 40, 40)); // Icon  background

        if (!csprite || !ctransform)
        {
            renderTexture.display();
            return renderTexture.getTexture();
        }

        sf::Texture* texture = AssetManager::Instance().LoadTexture(csprite->textureName);
        if (!texture)
        {
            LOG_ERROR("Could not load texture: " + csprite->textureName);
            renderTexture.display();
            return renderTexture.getTexture();
        }

        sf::Sprite sprite(*texture);
        sprite.setTextureRect(csprite->textureRect);
        sprite.setColor(csprite->color);
        sprite.setOrigin({csprite->textureRect.size.x * 0.5f, csprite->textureRect.size.y * 0.5f});
        sprite.setPosition({m_iconTextureSize * 0.5f, m_iconTextureSize * 0.5f});
        sprite.setRotation(sf::degrees(-ctransform->rotation)); // Negate rotation to match rendering

        float spriteWidth = csprite->size.x * ctransform->scale.x;
        float spriteHeight = csprite->size.y * ctransform->scale.y;
        float scaleX = spriteWidth / (m_iconTexturePadding * m_iconTextureSize);
        float scaleY = spriteHeight / (m_iconTexturePadding * m_iconTextureSize);
        float fitScale = std::max(scaleX, scaleY); // fit to icon size based on longest axis
        sprite.setScale({spriteWidth / (csprite->textureRect.size.x * fitScale), spriteHeight / (csprite->textureRect.size.y * fitScale)});

        renderTexture.draw(sprite);
        renderTexture.display();

        return renderTexture.getTexture();
    }

    void UpdateEntityTemplate(Json& entityJson, const std::string& name)
    {
        auto* templatePtr = GetTemplateByName(name);
        if (!templatePtr)
        {
            LOG_WARNING("Tried to update non-existent template: " + name);
            return;
        }

        templatePtr->entityJson = entityJson;
        JsonUtility::SaveJsonObjectToFile(entityJson, storageFolder + name + ".json");

        const sf::Texture texture = GenerateEntityTemplateIconTexture(entityJson);
        templatePtr->iconTexture = std::move(texture);
    }

    void DeleteEntityTemplate(const std::string& name)
    {
        auto it = m_templateMap.find(name);
        if (it == m_templateMap.end())
        {
            LOG_WARNING("Tried to delete non-existent template: " + name);
            return;
        }

        FileHelper::DeleteFile(storageFolder + name + ".json");
        m_templateMap.erase(it);
    }

    void ChangeEntityTemplateName(const std::string& oldName, const std::string& newName)
    {
        if (m_templateMap.contains(newName))
        {
            LOG_WARNING("Template already exists: " + newName);
            return;
        }

        auto pair = m_templateMap.extract(oldName);
        if (!pair)
        {
            LOG_WARNING("Template not found: " + oldName);
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

                const sf::Texture texture = GenerateEntityTemplateIconTexture(entityJson);
                auto newTemplate = std::make_unique<EntityTemplate>(std::move(texture), filename, entityJson);
                m_templateMap.emplace(filename, std::move(newTemplate));
            }
        }
    }
};
