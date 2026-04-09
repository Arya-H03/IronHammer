#pragma once

#include "assets/AssetManager.h"
#include "core/reflection/ComponentReflection.h"
#include "core/utils/Vect2.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <cfloat>
#include <cstddef>
#include <string>

struct CSprite
{
    static constexpr const char* name = "Sprite";

    sf::IntRect textureRect = sf::IntRect({0, 0}, {32, 32});
    const sf::Texture* texturePtr = nullptr;
    std::string textureName = "";
    sf::Color color = sf::Color::White;
    Vect2f size = Vect2f(32, 32);

    CSprite() = default;
    CSprite(const std::string& texName, Vect2f sz, sf::IntRect texRect, sf::Color col = sf::Color::White)
        : textureName(texName), size(sz), textureRect(texRect), color(col)
    {
        texturePtr = AssetManager::Instance().LoadTexture(textureName);
    }

    void OnAfterDeserialize()
    {
        texturePtr = textureName.empty() ? nullptr : AssetManager::Instance().LoadTexture(textureName);
    }
};

template <>
struct Reflect<CSprite>
{
    static constexpr auto fields =
        std::make_tuple(Descriptor{"textureName", &CSprite::textureName, true}, Descriptor{"size", &CSprite::size, true},
                        Descriptor{"textureRect", &CSprite::textureRect, true}, Descriptor{"color", &CSprite::color, true});
};

struct CShape
{
    static constexpr const char* name = "Shape";

    sf::Color fillColor = sf::Color::White;    // sf::Color() == Black, mismatched Reset()
    sf::Color outlineColor = sf::Color::White; // same
    size_t points = 3;
    float radius = 10.f;
    float outlineThickness = 1.f;

    CShape() = default;
    CShape(size_t pts, const sf::Color& fill, const sf::Color& outline, float rad, float thickness)
        : points(pts), fillColor(fill), outlineColor(outline), radius(rad), outlineThickness(thickness)
    {
    }
};

template <>
struct Reflect<CShape>
{
    static constexpr auto fields =
        std::make_tuple(Descriptor{"points", &CShape::points, true}, Descriptor{"radius", &CShape::radius, true},
                        Descriptor{"outlineThickness", &CShape::outlineThickness, true}, Descriptor{"fillColor", &CShape::fillColor, true},
                        Descriptor{"outlineColor", &CShape::outlineColor, true});
};

struct CText
{
    static constexpr const char* name = "Text";

    std::string content = "";
    sf::Color textColor = sf::Color::White;
    Vect2f offset = Vect2f(0, 0);
    float fontSize = 12.f;

    CText() = default;
    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size)
        : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }
};

template <>
struct Reflect<CText>
{
    static constexpr auto fields =
        std::make_tuple(Descriptor{"content", &CText::content, true}, Descriptor{"fontSize", &CText::fontSize, true},
                        Descriptor{"offset", &CText::offset, true}, Descriptor{"textColor", &CText::textColor, true});
};

struct CNotDrawable
{
    static constexpr const char* name = "NotDrawable";

    CNotDrawable() = default;
};

template <>
struct Reflect<CNotDrawable>
{
    static constexpr auto fields = std::make_tuple();
};
