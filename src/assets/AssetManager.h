#pragma once
#include "core/utils/Debug.h"
#include <SFML/Graphics/Texture.hpp>
#include <memory>
#include <SFML/Graphics.hpp>
#include <string>

class AssetManager
{
  private:

    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> m_textures;

  public:

    AssetManager()
    {
        // This is for testing
        CreateTexture("Square", "assets/Square.png");
    }

    static AssetManager& Instance()
    {
        static AssetManager instance;
        return instance;
    }

    void CreateTexture(const std::string& name, const std::string& filePath)
    {
        if (m_textures.contains(name))
        {
            Log_Error("Texture already exists: " + name);
            return;
        }

        std::unique_ptr<sf::Texture> texture = std::make_unique<sf::Texture>();

        if (!texture->loadFromFile(filePath))
        {
            Log_Error("Could not load texture: " + filePath);
            return;
        }

        m_textures.emplace(name, std::move(texture));
    }

    sf::Texture* LoadTexture(const std::string& name)
    {
        auto it = m_textures.find(name);
        if (it == m_textures.end())
        {
            Log_Error("Could not file texture: " + name);
            return nullptr;
        }
        return it->second.get();
    }

    void Clear() { m_textures.clear(); }
};
