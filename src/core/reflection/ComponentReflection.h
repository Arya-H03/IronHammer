#pragma once
#include "core/utils/Vect2.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <nlohmann/json.hpp>
#include <tuple>

using Json = nlohmann::json;

template <typename Component>
struct Reflect;

template <typename Owner, typename Field>
struct Descriptor
{
    const char* name;
    Field Owner::*ptr;
    bool canBeMolded;
};

template <typename Component, typename Visitor>
void ReflectVisit(Component& component, Visitor& visitor)
{
    std::apply([&](auto... descriptor) { (visitor.Field(descriptor.name, component.*descriptor.ptr), ...); }, Reflect<Component>::fields);
}

template <typename Component, typename Visitor>
void ReflectVisit(Component& component, Component& mold, Visitor& visitor)
{
    std::apply([&](auto... descriptor) { (visitor.Field(component.*descriptor.ptr, mold.*descriptor.ptr, descriptor.canBeMolded), ...); },
               Reflect<Component>::fields);
}

struct SerializeVisitor
{
    Json& json;

    // Primitives
    template <typename T>
    void Field(const char* name, const T& value)
    {
        json[name] = value;
    }

    // Vect2
    template <typename T>
    void Field(const char* name, const Vect2<T>& value)
    {
        json[name] = {{"x", value.x}, {"y", value.y}};
    }

    // SFML types
    void Field(const char* name, const sf::IntRect& value)
    {
        json[name] = {value.position.x, value.position.y, value.size.x, value.size.y};
    }

    void Field(const char* name, const sf::Color& value)
    {
        json[name] = {value.r, value.g, value.b, value.a};
    }
};

struct DeSerializeVisitor
{
    const Json& json;

    // Primitives
    // Won't work with const char*
    // Consider making a new specialization for it ????
    template <typename T>
    void Field(const char* name, T& value)
    {
        if (!json.contains(name)) return;

        value = json[name].get<T>();
    }

    // Vect2
    template <typename T>
    void Field(const char* name, Vect2<T>& value)
    {
        if (!json.contains(name)) return;

        value.x = json[name].value("x", 0);
        value.y = json[name].value("y", 0);
    }

    // SFML types
    void Field(const char* name, sf::IntRect& value)
    {
        if (!json.contains(name)) return;

        const auto& arr = json[name];
        value = sf::IntRect({arr[0], arr[1]}, {arr[2], arr[3]});
    }

    void Field(const char* name, sf::Color& value)
    {
        if (!json.contains(name)) return;

        const auto& arr = json[name];
        value = sf::Color(arr[0], arr[1], arr[2], arr[3]);
    }
};

struct MoldingVisitor
{

    // Primitives
    template <typename T>
    void Field(T& value, const T& moldValue, bool canBeMolded)
    {
        if (!canBeMolded) return;
        value = moldValue;
    }

    // Vect2
    template <typename T>
    void Field(Vect2<T>& value, const Vect2<T>& moldValue, bool canBeMolded)
    {
        if (!canBeMolded) return;
        value = moldValue;
    }

    // SFML types
    void Field(sf::IntRect& value, const sf::IntRect& moldValue, bool canBeMolded)
    {
        if (!canBeMolded) return;
        value = moldValue;
    }

    void Field(sf::Color& value, const sf::Color& moldValue, bool canBeMolded)
    {
        if (!canBeMolded) return;
        value = moldValue;
    }
};
