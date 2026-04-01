#pragma once
#include "core/utils/Debug.h"
#include "imgui.h"

#include <memory>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>

class AssetManager
{
private:
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<sf::Font>>    m_fonts;

public:
    AssetManager()
    {
        // This is for testing
        CreateTexture("Square", "assets/Square.png");
        CreateTexture("PlayBtn", "assets/play-button.png");
        CreateTexture("PauseBtn", "assets/pause-button.png");
        CreateTexture("CrossBtn", "assets/cross-button.png");
        CreateTexture("ResetBtn", "assets/reset-button.png");
        CreateTexture("RightArrowSharp", "assets/right-arrow-sharp.png");
        CreateTexture("UpArrowSharp", "assets/up-arrow-sharp.png");
        CreateTexture("RightArrowFlat", "assets/right-arrow-flat.png");
        CreateTexture("UpArrowFlat", "assets/up-arrow-flat.png");

        CreateFont("Default", "assets/font.otf");
    }

    AssetManager(const AssetManager&)                = delete;
    AssetManager& operator=(const AssetManager&)     = delete;
    AssetManager(AssetManager&&) noexcept            = delete;
    AssetManager& operator=(AssetManager&&) noexcept = delete;

    static AssetManager& Instance()
    {
        static AssetManager instance;
        return instance;
    }

    void CreateTexture(const std::string& name, const std::string& filePath)
    {
        if (m_textures.contains(name)) {
            LOG_ERROR("Texture already exists: " + name);
            return;
        }

        std::unique_ptr<sf::Texture> texture = std::make_unique<sf::Texture>();

        if (!texture->loadFromFile(filePath)) {
            LOG_ERROR("Could not load texture: " + filePath);
            return;
        }

        m_textures.emplace(name, std::move(texture));
    }

    sf::Texture* LoadTexture(const std::string& name)
    {
        auto it = m_textures.find(name);
        if (it == m_textures.end()) {
            LOG_ERROR("Could not find texture: " + name);
            return nullptr;
        }
        return it->second.get();
    }

    ImTextureID GetTextureID(const std::string& name)
    {
        auto it = m_textures.find(name);
        if (it == m_textures.end()) {
            LOG_ERROR("Could not find texture: " + name);
            return -1;
        }
        return it->second.get()->getNativeHandle();
    }

    void CreateFont(const std::string& name, const std::string& filePath)
    {
        if (m_fonts.contains(name)) {
            LOG_ERROR("Font already exists: " + name);
            return;
        }

        std::unique_ptr<sf::Font> font = std::make_unique<sf::Font>();

        if (!font->openFromFile(filePath)) {
            LOG_ERROR("Could not load font: " + filePath);
            return;
        }

        m_fonts.emplace(name, std::move(font));
    }

    sf::Font* LoadFont(const std::string& name)
    {
        auto it = m_fonts.find(name);
        if (it == m_fonts.end()) {
            LOG_ERROR("Could not find font: " + name);
            return nullptr;
        }
        return it->second.get();
    }

    void Clear() { m_textures.clear(); }
};
