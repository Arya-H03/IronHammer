#pragma once
#include <SFML/Graphics/CircleShape.hpp>
#include "core/utils/Vect2.hpp"
#include <format>
#include <string>

struct CTransform
{
    Vect2f position;
    Vect2f rotation;
    Vect2f scale;

    const std::string GetDescription() const
    {
        return std::format("position ({},{}) rotation ({},{}) scale ({},{})",
                           position.x,
                           position.y,
                           rotation.x,
                           rotation.y,
                           scale.x,
                           scale.y);
    }
};

struct CMovement
{
    Vect2f velocity;
    float speed;
    const std::string GetDescription() const { return std::format("Velocity:({},{}) Speed:{}", velocity.x, velocity.y,speed); }
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
