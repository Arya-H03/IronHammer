#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <format>
#include <SFML/Graphics/CircleShape.hpp>
#include <string>
#include "core/utils/Vect2.hpp"
#include "core/utils/CustomTypes.hpp"

struct CTransform
{
    Vect2f position;
    float rotation;
    Vect2f scale;

    const std::string GetDescription() const
    {
        return std::format("position ({},{}) rotation ({}) scale ({},{})",
                           position.x,
                           position.y,
                           rotation,
                           scale.x,
                           scale.y);
    }
};

struct CMovement
{
    Vect2f velocity;
    float speed;
    const std::string GetDescription() const
    {
        return std::format("Velocity:({},{}) Speed:{}", velocity.x, velocity.y, speed);
    }
};

struct CShape
{
    float radius;
    size_t points;
    sf::Color fillColor;
    sf::Color outlineColor;
    float outlineThickness;

    const std::string GetDescription() const { return std::format("Shaspe"); }
};

struct CCollider
{
    BoundingBox boundingBox;
    bool isTrigger = false;

    const std::string GetDescription() const
    {
        return std::format("Bounding Box: origin({}, {}) , bounds({}, {}) , isTrigger: {}",
                           boundingBox.origin.x,
                           boundingBox.origin.y,
                           boundingBox.bounds.x,
                           boundingBox.bounds.y,
                           isTrigger);
    }
};

struct CText
{
    std::string content;
    sf::Color textColor;

    const std::string GetDescription() const { return content; }
};
