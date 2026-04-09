#pragma once
#include "core/saving/JsonUtility.h"
#include "ecs/common/ECSCommon.h"

#include <SFML/Graphics/Texture.hpp>

static inline constexpr const uint16_t MoldIconTextureSize = 64;
static inline constexpr const float MoldIconTexturePadding = 0.8f;

struct Mold
{
    sf::Texture iconTexture;
    std::string entityName;
    Json entityJson;
    std::vector<Entity> derivedEntities;

    Mold() = delete;
    Mold(sf::Texture tex, const std::string& name, const Json& json) : iconTexture(std::move(tex)), entityName(name), entityJson(json)
    {
    }
};
