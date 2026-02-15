#pragma once
#include <SFML/Graphics/CircleShape.hpp>
#include <format>
#include <string>

struct CPosition
{
    float x, y, z;

    const std::string GetDescription() const { return std::format("({}, {}, {})", x, y, z); }
};

struct CVelocity
{
    float x, y, z;

    const std::string GetDescription() const { return std::format("({}, {}, {})", x, y, z); }
};

struct CRotation
{
    float x, y, z;

    const std::string GetDescription() const { return std::format("({}, {}, {})", x, y, z); }
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
