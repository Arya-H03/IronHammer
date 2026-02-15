#pragma once
#include <SFML/Graphics/CircleShape.hpp>
#include <format>
#include <string>


struct CPosition
{
    float x, y, z;

    const std::string GetDescription() const
    {
        return std::format("({}, {}, {})",x,y,z);
    }
};

struct CVelocity
{
    float x, y, z;

    const std::string GetDescription() const
    {
        return std::format("({}, {}, {})",x,y,z);
    }
};

struct CRotation
{
    float x, y, z;

    const std::string GetDescription() const
    {
        return std::format("({}, {}, {})",x,y,z);
    }
};

struct CShape
{
   	sf::CircleShape shape;

	CShape(float radius, size_t points, const sf::Color& fillColor, const sf::Color& outlineColor, float outlineThickness)
	{
	shape.setRadius(radius);
	shape.setPointCount(points);
	shape.setFillColor(fillColor);
	shape.setOutlineColor(outlineColor);
	shape.setOutlineThickness(outlineThickness);
	shape.setOrigin({ radius, radius });
	}

	const std::string GetDescription() const
    {
        return std::format("Shape");
    }
};
