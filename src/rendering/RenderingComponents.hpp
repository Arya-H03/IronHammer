#pragma once

#include "assets/AssetManager.h"
#include "core/reflection/ComponentReflection.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"

#include <cstddef>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>

using namespace EntityInspectorHelpers;

struct CSprite
{
    sf::IntRect        textureRect = sf::IntRect({0, 0}, {32, 32});
    sf::Color          color       = sf::Color::White;
    const sf::Texture* texturePtr  = nullptr;
    std::string        textureName = "";
    Vect2f             size        = Vect2f(32, 32);

    static constexpr const char* name = "Sprite";

    CSprite() = default;
    CSprite(const std::string& texName, Vect2f sz, sf::IntRect texRect, sf::Color col = sf::Color::White)
        : textureName(texName), size(sz), textureRect(texRect), color(col)
    {
        texturePtr = AssetManager::Instance().LoadTexture(textureName);
    }

    REGISTER_COMPONENT(CSprite);

    void OnAfterDeserialize()
    {
        texturePtr = textureName.empty() ? nullptr : AssetManager::Instance().LoadTexture(textureName);
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CSprite>("Sprite", ptr, RemoveComponentCallback, [this] { *this = CSprite{}; }, isDirty)) {
            if (ImGui::BeginTable("CSpriteTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                TableNextField("Texture");
                EntityInspectorHelpers::InputText("##TextureName", textureName, isDirty);
                TableNextField("Size");
                EntityInspectorHelpers::DragFloat2("##SpriteSizeX", &size.x, "##SpriteSizeY", &size.y, 0.1f, isDirty);
                TableNextField("Color");
                EntityInspectorHelpers::ColorEdit4("##SpriteColor", color, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct Reflect<CSprite>
{
    static constexpr auto fields =
        std::make_tuple(std::pair{"textureName", &CSprite::textureName}, std::pair{"size", &CSprite::size},
                        std::pair{"textureRect", &CSprite::textureRect}, std::pair{"color", &CSprite::color});
};

struct CShape
{
    size_t    points           = 3;
    sf::Color fillColor        = sf::Color::White; // sf::Color() == Black, mismatched Reset()
    sf::Color outlineColor     = sf::Color::White; // same
    float     radius           = 10.f;
    float     outlineThickness = 1.f;

    static constexpr const char* name = "Shape";

    CShape() = default;
    CShape(size_t pts, const sf::Color& fill, const sf::Color& outline, float rad, float thickness)
        : points(pts), fillColor(fill), outlineColor(outline), radius(rad), outlineThickness(thickness)
    {
    }

    REGISTER_COMPONENT(CShape);

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CShape>("Shape", ptr, RemoveComponentCallback, [this] { *this = CShape{}; }, isDirty)) {
            if (ImGui::BeginTable("CShapeTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                TableNextField("Points");
                EntityInspectorHelpers::DragScalar("##Points", &points, isDirty);
                TableNextField("Radius");
                EntityInspectorHelpers::DragFloatWithLimits("##ShapeRadius", &radius, 0.1f, 0.f, 100.f, isDirty);
                TableNextField("Outline Thickness");
                EntityInspectorHelpers::DragFloatWithLimits("##ShapeThickness", &outlineThickness, 0.1f, 0.f, 100.f,
                                                            isDirty);
                TableNextField("Fill Color");
                EntityInspectorHelpers::ColorEdit4("##ShapeFill", fillColor, isDirty);
                TableNextField("Outline Color");
                EntityInspectorHelpers::ColorEdit4("##ShapeOutline", outlineColor, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct Reflect<CShape>
{
    static constexpr auto fields =
        std::make_tuple(std::pair{"points", &CShape::points}, std::pair{"radius", &CShape::radius},
                        std::pair{"outlineThickness", &CShape::outlineThickness}, std::pair{"fillColor", &CShape::fillColor},
                        std::pair{"outlineColor", &CShape::outlineColor});
};

struct CText
{
    std::string content   = "";
    sf::Color   textColor = sf::Color::White;
    Vect2f      offset    = Vect2f(0, 0);
    float       fontSize  = 12.f;

    static constexpr const char* name = "Text";

    CText() = default;
    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size)
        : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }
    REGISTER_COMPONENT(CText);

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CText>("Text", ptr, RemoveComponentCallback, [this] { *this = CText{}; }, isDirty)) {
            if (ImGui::BeginTable("CTextTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                TableNextField("Content");
                EntityInspectorHelpers::InputText("##TextContent", content, isDirty);
                TableNextField("Font Size");
                EntityInspectorHelpers::DragFloatWithLimits("##TextSize", &fontSize, 1.f, 1.f, 200.f, isDirty);
                TableNextField("Offset");
                EntityInspectorHelpers::DragFloat2("##TextOffsetX", &offset.x, "##TextOffsetY", &offset.y, 0.1f, isDirty);
                TableNextField("Text Color");
                EntityInspectorHelpers::ColorEdit4("##TextColor", textColor, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct Reflect<CText>
{
    static constexpr auto fields =
        std::make_tuple(std::pair{"content", &CText::content}, std::pair{"fontSize", &CText::fontSize},
                        std::pair{"offset", &CText::offset}, std::pair{"textColor", &CText::textColor});
};

struct CNotDrawable
{
    static constexpr const char* name = "NotDrawable";

    CNotDrawable() = default;
    REGISTER_COMPONENT(CNotDrawable);

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CNotDrawable>(name, ptr, RemoveComponentCallback, [this] { *this = CNotDrawable{}; }, isDirty);
    }
};

template <>
struct Reflect<CNotDrawable>
{
    static constexpr auto fields = std::make_tuple();
};
