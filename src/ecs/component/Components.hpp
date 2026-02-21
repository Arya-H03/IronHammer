#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <format>
#include <SFML/Graphics/CircleShape.hpp>
#include <string>
#include "core/utils/Vect2.hpp"

struct CTransform
{
    Vect2f position;
    Vect2f scale;
    float rotation;

    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) { }

    std::string GetDescription() const
    {
        return std::format(
            "position ({},{}) rotation ({}) scale ({},{})", position.x, position.y, rotation, scale.x, scale.y);
    }
};

struct CMovement
{
    Vect2f velocity;
    float speed;

    CMovement(const Vect2f& vel, float spd) : velocity(vel), speed(spd) { }

    std::string GetDescription() const { return std::format("Velocity:({},{}) Speed:{}", velocity.x, velocity.y, speed); }
};

struct CShape
{
    size_t points;
    sf::Color fillColor;
    sf::Color outlineColor;
    float radius;
    float outlineThickness;

    CShape(size_t pts, const sf::Color& fill, const sf::Color& outline, float rad, float thickness)
        : points(pts), fillColor(fill), outlineColor(outline), radius(rad), outlineThickness(thickness)
    {
    }

    std::string GetDescription() const { return std::format("Shape: points({}) radius({})", points, radius); }
};

struct CCollider
{
    Vect2f size;
    Vect2f halfSize;
    Vect2f offset;
    bool isTrigger;

    CCollider(const Vect2f& sz, const Vect2f& off, bool trigger = false)
        : size(sz), halfSize(sz.x * 0.5f, sz.y * 0.5f), offset(off), isTrigger(trigger)
    {
    }

    std::string GetDescription() const
    {
        return std::format(
            "Size: ({}, {}) , offset: ({}, {}) , isTrigger: {}", size.x, size.y, offset.x, offset.y, isTrigger);
    }
};

struct CText
{
    std::string content;
    sf::Color textColor;
    Vect2f offset;
    float fontSize;

    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size)
        : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }

    std::string GetDescription() const { return content; }
};

struct CNotDrawable
{
    const std::string GetDescription() const { return ""; }
};
