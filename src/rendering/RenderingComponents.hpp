#pragma once

#include "assets/AssetManager.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"

#include <cstddef>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>

using namespace EntityInspectorHelpers;
using Json = nlohmann::json;

struct CSprite
{
    const sf::Texture* texture     = nullptr;
    std::string        textureName = "";
    Vect2f             size        = Vect2f(32, 32);
    sf::IntRect        textureRect = sf::IntRect({0, 0}, {32, 32});
    sf::Color          color       = sf::Color::White;

    static constexpr const char* name = "Sprite";

    CSprite() = default;
    CSprite(const std::string& texName, Vect2f sz, sf::IntRect texRect, sf::Color col = sf::Color::White)
        : textureName(texName), size(sz), textureRect(texRect), color(col)
    {
        texture = AssetManager::Instance().LoadTexture(textureName);
    }

    REGISTER_COMPONENT(CSprite);

    void Reset()
    {
        textureName = "";
        texture     = nullptr;
        size        = Vect2f(32, 32);
        textureRect = sf::IntRect({0, 0}, {32, 32});
        color       = sf::Color::White;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CSprite>("Sprite", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty)) {
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

inline void to_json(Json& j, const CSprite& c)
{
    j                = Json::object();
    j["textureName"] = c.textureName;
    j["size"]        = {c.size.x, c.size.y};
    j["textureRect"] = {c.textureRect.position.x, c.textureRect.position.y, c.textureRect.size.x, c.textureRect.size.y};
    j["color"]       = {c.color.r, c.color.g, c.color.b, c.color.a};
}

inline void from_json(const Json& j, CSprite& c)
{
    c.textureName = j.value("textureName", "");
    c.texture     = c.textureName.empty() ? nullptr : AssetManager::Instance().LoadTexture(c.textureName);
    auto sizeArr  = j.value("size", std::vector<float>{32.f, 32.f});
    c.size        = Vect2f(sizeArr[0], sizeArr[1]);

    auto texRect  = j.value("textureRect", std::vector<int>{0, 0, 32, 32});
    c.textureRect = sf::IntRect({texRect[0], texRect[1]}, {texRect[2], texRect[3]});

    auto col = j.value("color", std::vector<uint8_t>{255, 255, 255, 255});
    c.color  = sf::Color(col[0], col[1], col[2], col[3]);
}

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

    void Reset()
    {
        points           = 3;
        radius           = 10.f;
        outlineThickness = 1.f;
        fillColor        = sf::Color::White;
        outlineColor     = sf::Color::White;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CShape>("Shape", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty)) {
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

inline void to_json(Json& j, const CShape& c)
{
    j = {{"points", c.points},
         {"radius", c.radius},
         {"outlineThickness", c.outlineThickness},
         {"fillColor", {c.fillColor.r, c.fillColor.g, c.fillColor.b, c.fillColor.a}},
         {"outlineColor", {c.outlineColor.r, c.outlineColor.g, c.outlineColor.b, c.outlineColor.a}}};
}

inline void from_json(const Json& j, CShape& c)
{
    c.points           = j.value("points", 3);
    c.radius           = j.value("radius", 10.f);
    c.outlineThickness = j.value("outlineThickness", 1.f);

    auto fill   = j.value("fillColor", std::vector<uint8_t>{255, 255, 255, 255});
    c.fillColor = sf::Color(fill[0], fill[1], fill[2], fill[3]);

    auto outline   = j.value("outlineColor", std::vector<uint8_t>{255, 255, 255, 255});
    c.outlineColor = sf::Color(outline[0], outline[1], outline[2], outline[3]);
}

struct CText
{
    std::string                  content   = "";
    sf::Color                    textColor = sf::Color::White;
    Vect2f                       offset    = Vect2f(0, 0);
    float                        fontSize  = 12.f;
    static constexpr const char* name      = "Text";

    CText() = default;
    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size)
        : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }
    REGISTER_COMPONENT(CText);

    void Reset()
    {
        content   = "";
        textColor = sf::Color::White;
        offset    = Vect2f(0, 0);
        fontSize  = 12.f;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CText>("Text", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty)) {
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

inline void to_json(Json& j, const CText& c)
{
    j = {{"content", c.content},
         {"fontSize", c.fontSize},
         {"offset", {{"x", c.offset.x}, {"y", c.offset.y}}},
         {"textColor", {c.textColor.r, c.textColor.g, c.textColor.b, c.textColor.a}}};
}

inline void from_json(const Json& j, CText& c)
{
    c.content  = j.value("content", std::string{""});
    c.fontSize = j.value("fontSize", 12.f);
    c.offset.x = j["offset"].value("x", 0.f);
    c.offset.y = j["offset"].value("y", 0.f);

    auto col    = j.value("textColor", std::vector<uint8_t>{255, 255, 255, 255});
    c.textColor = sf::Color(col[0], col[1], col[2], col[3]);
}

struct CNotDrawable
{
    static constexpr const char* name = "NotDrawable";

    CNotDrawable() = default;
    REGISTER_COMPONENT(CNotDrawable);

    void Reset() {};

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CNotDrawable>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CNotDrawable&) { j = Json::object(); }
inline void from_json(const Json&, CNotDrawable&) {}
