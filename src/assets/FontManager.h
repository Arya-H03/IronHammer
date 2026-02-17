#pragma once
#include <SFML/Graphics/Font.hpp>
#include <cassert>
#include <string>

class FontManager
{
  private:
    inline static const std::string fontPath = "assets/font.otf";
    inline static sf::Font m_font;

  public:
    static void InitializeFont()
    {
        if (!m_font.openFromFile(fontPath))
        {
            throw std::runtime_error("Failed to load font at" + fontPath);
        }
    }

    static const sf::Font& GetFont() { return m_font; }
};
