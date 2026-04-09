#pragma once
#include "assets/AssetManager.h"
#include "core/CoreComponents.hpp"
#include "core/saving/JsonUtility.h"
#include "core/utils/Debug.h"
#include "core/utils/FileHelper.h"
#include "ecs/World.h"
#include "ecs/archetype/ArchetypeRegistry.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/query/Query.hpp"
#include "mold/Mold.h"
#include "rendering/RenderingComponents.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Angle.hpp>
#include <algorithm>
#include <filesystem>
#include <string>
#include <utility>

class MoldManager
{
  private:
    const std::string moldsStorageFolder = "src/assets/molds/";
    std::unordered_map<std::string, std::unique_ptr<Mold>> m_moldMap;

  public:
    MoldManager() = default;

    const std::unordered_map<std::string, std::unique_ptr<Mold>>& GetMoldMap() const
    {
        return m_moldMap;
    }

    Mold* GetMoldByName(const std::string& name)
    {
        auto it = m_moldMap.find(name);
        if (it == m_moldMap.end()) return nullptr;
        return it->second.get();
    }

    bool CreateMold(World& world, Entity entity, const EntityStorageLocation& entityLocation, const std::string& name)
    {
        if (m_moldMap.contains(name))
        {
            LOG_WARNING("Tried to create an Entity Template with an already existing name: " + name);
            return false;
        }

        Json entityJson = world.SerializeEntity(entityLocation);

        const sf::Texture texture = GenerateMoldIconTextureFromWorld(world, entity);
        m_moldMap.emplace(name, std::make_unique<Mold>(std::move(texture), name, entityJson));

        JsonUtility::SaveJsonObjectToFile(entityJson, moldsStorageFolder + name + ".json");

        return true;
    }

    const sf::Texture GenerateMoldIconTextureFromWorld(World& world, Entity entity)
    {
        CSprite* csprite = world.TryGetComponent<CSprite>(entity);
        CTransform* ctransform = world.TryGetComponent<CTransform>(entity);

        sf::RenderTexture renderTexture({MoldIconTextureSize, MoldIconTextureSize});
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
        sprite.setPosition({MoldIconTextureSize * 0.5f, MoldIconTextureSize * 0.5f});
        sprite.setRotation(sf::degrees(-ctransform->rotation)); // Negate rotation to match rendering

        float spriteWidth = csprite->size.x * ctransform->scale.x;
        float spriteHeight = csprite->size.y * ctransform->scale.y;
        float scaleX = spriteWidth / (MoldIconTexturePadding * MoldIconTextureSize);
        float scaleY = spriteHeight / (MoldIconTexturePadding * MoldIconTextureSize);
        float fitScale = std::max(scaleX, scaleY); // fit to icon size based on longest axis
        sprite.setScale({spriteWidth / (csprite->textureRect.size.x * fitScale), spriteHeight / (csprite->textureRect.size.y * fitScale)});

        renderTexture.draw(sprite);
        renderTexture.display();

        return renderTexture.getTexture();
    }

    const sf::Texture GenerateMoldIconTextureFromJson(Json entityJson) const
    {
        PendingComponent pendingCSprite = ComponentRegistry::GetPendingComponentFromEntityJson<CSprite>(entityJson);
        CSprite* csprite = ComponentRegistry::GetComponentFromPending<CSprite>(pendingCSprite);

        PendingComponent pendingCTransform = ComponentRegistry::GetPendingComponentFromEntityJson<CTransform>(entityJson);
        CTransform* ctransform = ComponentRegistry::GetComponentFromPending<CTransform>(pendingCTransform);

        sf::RenderTexture renderTexture({MoldIconTextureSize, MoldIconTextureSize});
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
        sprite.setPosition({MoldIconTextureSize * 0.5f, MoldIconTextureSize * 0.5f});
        sprite.setRotation(sf::degrees(-ctransform->rotation)); // Negate rotation to match rendering

        float spriteWidth = csprite->size.x * ctransform->scale.x;
        float spriteHeight = csprite->size.y * ctransform->scale.y;
        float scaleX = spriteWidth / (MoldIconTexturePadding * MoldIconTextureSize);
        float scaleY = spriteHeight / (MoldIconTexturePadding * MoldIconTextureSize);
        float fitScale = std::max(scaleX, scaleY); // fit to icon size based on longest axis
        sprite.setScale({spriteWidth / (csprite->textureRect.size.x * fitScale), spriteHeight / (csprite->textureRect.size.y * fitScale)});

        renderTexture.draw(sprite);
        renderTexture.display();

        return renderTexture.getTexture();
    }

    void RemakeMoldWithJson(Json& entityJson, const std::string& name)
    {
        auto* templatePtr = GetMoldByName(name);
        if (!templatePtr)
        {
            LOG_WARNING("Tried to update non-existent template: " + name);
            return;
        }

        templatePtr->entityJson = entityJson;
        JsonUtility::SaveJsonObjectToFile(entityJson, moldsStorageFolder + name + ".json");

        const sf::Texture texture = GenerateMoldIconTextureFromJson(entityJson);
        templatePtr->iconTexture = std::move(texture);
    }

    void DeleteMold(const std::string& name)
    {
        auto it = m_moldMap.find(name);
        if (it == m_moldMap.end())
        {
            LOG_WARNING("Tried to delete non-existent template: " + name);
            return;
        }

        FileHelper::DeleteFile(moldsStorageFolder + name + ".json");
        m_moldMap.erase(it);
    }

    void ChangeMoldName(const std::string& oldName, const std::string& newName)
    {
        if (m_moldMap.contains(newName))
        {
            LOG_WARNING("Template already exists: " + newName);
            return;
        }

        auto pair = m_moldMap.extract(oldName);
        if (!pair)
        {
            LOG_WARNING("Template not found: " + oldName);
            return;
        }

        FileHelper::RenameFile(oldName, moldsStorageFolder + oldName + ".json", newName, moldsStorageFolder + newName + ".json");

        pair.key() = newName;
        pair.mapped()->entityName = newName;
        m_moldMap.insert(std::move(pair));
    }

    void LoadAllMoldsFromDisk()
    {
        for (const auto& entry : std::filesystem::directory_iterator(moldsStorageFolder))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                std::string filePath = entry.path().string();
                std::string filename = entry.path().stem().string();

                Json entityJson = JsonUtility::LoadJsonObjectFromFile(filePath);

                const sf::Texture texture = GenerateMoldIconTextureFromJson(entityJson);
                auto newTemplate = std::make_unique<Mold>(std::move(texture), filename, entityJson);
                m_moldMap.emplace(filename, std::move(newTemplate));
            }
        }
    }

    void FillAllMoldsWithDerivedEntities(World& world)
    {
        Query* moldDerivedQuery = world.Query<RequiredComponents<CMolded>>();
        moldDerivedQuery->ForEachWithEntity<CMolded>(
            [&](Entity entity, CMolded& moldDerived)
            {
                auto it = m_moldMap.find(moldDerived.moldName);
                if (it != m_moldMap.end())
                {
                    it->second->derivedEntities.push_back(entity);
                }
                else
                {
                    LOG_ERROR("Couldn't find the mold for entity.");
                }
            });
    }
};
