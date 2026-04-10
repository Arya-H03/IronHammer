#pragma once
#include "core/saving/JsonUtility.h"
#include "core/utils/Debug.h"
#include "ecs/common/ECSCommon.h"

#include <SFML/Graphics/Texture.hpp>
#include <cstdint>

static inline constexpr const uint16_t MoldIconTextureSize = 64;
static inline constexpr const float MoldIconTexturePadding = 0.8f;

struct Mold
{
    sf::Texture iconTexture;
    std::string moldName;
    Json moldJson;
    std::vector<Entity> derivedEntities;

    Mold() = delete;
    Mold(sf::Texture tex, const std::string& name, const Json& json) : iconTexture(std::move(tex)), moldName(name), moldJson(json)
    {
    }

    void RemoveDerivedEntity(Entity entity)
    {
        size_t toBeRemovedIndex = SIZE_MAX;
        for (size_t i = 0; i < derivedEntities.size(); ++i)
        {
            if (derivedEntities[i] == entity)
            {
                toBeRemovedIndex = i;
                break;
            }
        }
        if (toBeRemovedIndex != SIZE_MAX)
        {
            derivedEntities[toBeRemovedIndex] = derivedEntities.back();
            derivedEntities.pop_back();
        }
        else
        {
            LOG_WARNING("Tried to remove an entity that doesn't exist in derived entites");
        }
    }
};
