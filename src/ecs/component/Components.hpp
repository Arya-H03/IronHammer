#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Mouse.hpp>
#include <string>
#include <nlohmann/json.hpp>
#include "assets/AssetManager.h"
#include "ecs/entity/EntityInspectorHelper.h"
#include "core/utils/Vect2.hpp"
#include "imgui.h"
#include "ecs/component/ComponentRegistry.hpp"

using namespace EntityInspectorHelpers;
using Json = nlohmann::json;

struct CTransform
{
    Vect2f position = Vect2f(0, 0);
    Vect2f scale = Vect2f(1, 1);
    float rotation = 0.f;
    static constexpr const char* name = "Transform";

    CTransform() = default;
    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) { }
    REGISTER_COMPONENT(CTransform);

    void Reset()
    {
        position = Vect2f(0, 0);
        scale = Vect2f(1, 1);
        rotation = 0.f;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CTransform>("Transform", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty))
        {
            if (ImGui::BeginTable("CTransformTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Position");
                EntityInspectorHelpers::DragFloat2("##PosX", &position.x, "##PosY", &position.y, 0.1f, isDirty);
                TableNextField("Scale");
                EntityInspectorHelpers::DragFloat2("##ScaleX", &scale.x, "##ScaleY", &scale.y, 0.1f, isDirty);
                TableNextField("Rotation");
                EntityInspectorHelpers::DragFloatWithLimits("##Rotation", &rotation, 0.5f, -360.f, 360.f, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

inline void to_json(Json& json, const CTransform& c)
{
    json = { { "position", { { "x", c.position.x }, { "y", c.position.y } } },
        { "scale", { { "x", c.scale.x }, { "y", c.scale.y } } },
        { "rotation", c.rotation } };
}

inline void from_json(const Json& json, CTransform& c)
{
    c.position.x = json["position"].value("x", 0.f);
    c.position.y = json["position"].value("y", 0.f);
    c.scale.x = json["scale"].value("x", 1.f);
    c.scale.y = json["scale"].value("y", 1.f);
    c.rotation = json["rotation"];
}

struct CMovement
{
    float speed = 0.f;
    static constexpr const char* name = "Movement";

    CMovement() = default;
    CMovement(float spd) : speed(spd) { }
    REGISTER_COMPONENT(CMovement);

    void Reset() { speed = 0.f; }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CMovement>("Movement", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty))
        {
            if (ImGui::BeginTable("CMovementTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Speed");
                EntityInspectorHelpers::DragFloatWithLimits("##MoveSpeed", &speed, 0.1f, 0.f, 10000.f, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

inline void to_json(Json& j, const CMovement& c) { j = { { "speed", c.speed } }; }
inline void from_json(const Json& j, CMovement& c) { c.speed = j.value("speed", 0.f); }

struct CSprite
{
    const sf::Texture* texture = nullptr;
    std::string textureName = "";
    Vect2f size = Vect2f(32, 32);
    sf::IntRect textureRect = sf::IntRect({ 0, 0 }, { 32, 32 });
    sf::Color color = sf::Color::White;

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
        texture = nullptr;
        size = Vect2f(32, 32);
        textureRect = sf::IntRect({ 0, 0 }, { 32, 32 });
        color = sf::Color::White;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CSprite>("Sprite", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty))
        {
            if (ImGui::BeginTable("CSpriteTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
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
    j = Json::object();
    j["textureName"] = c.textureName;
    j["size"] = { c.size.x, c.size.y };
    j["textureRect"] = {
        c.textureRect.position.x, c.textureRect.position.y, c.textureRect.size.x, c.textureRect.size.y
    };
    j["color"] = { c.color.r, c.color.g, c.color.b, c.color.a };
}

inline void from_json(const Json& j, CSprite& c)
{
    c.textureName = j.value("textureName", "");
    c.texture = c.textureName.empty() ? nullptr : AssetManager::Instance().LoadTexture(c.textureName);
    auto sizeArr = j.value("size", std::vector<float> { 32.f, 32.f });
    c.size = Vect2f(sizeArr[0], sizeArr[1]);

    auto texRect = j.value("textureRect", std::vector<int> { 0, 0, 32, 32 });
    c.textureRect = sf::IntRect({ texRect[0], texRect[1] }, { texRect[2], texRect[3] });

    auto col = j.value("color", std::vector<uint8_t> { 255, 255, 255, 255 });
    c.color = sf::Color(col[0], col[1], col[2], col[3]);
}

struct CShape
{
    size_t points = 3;
    sf::Color fillColor = sf::Color::White;    // sf::Color() == Black, mismatched Reset()
    sf::Color outlineColor = sf::Color::White; // same
    float radius = 10.f;
    float outlineThickness = 1.f;
    static constexpr const char* name = "Shape";

    CShape() = default;
    CShape(size_t pts, const sf::Color& fill, const sf::Color& outline, float rad, float thickness)
        : points(pts), fillColor(fill), outlineColor(outline), radius(rad), outlineThickness(thickness)
    {
    }
    REGISTER_COMPONENT(CShape);

    void Reset()
    {
        points = 3;
        radius = 10.f;
        outlineThickness = 1.f;
        fillColor = sf::Color::White;
        outlineColor = sf::Color::White;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CShape>("Shape", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty))
        {
            if (ImGui::BeginTable("CShapeTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Points");
                EntityInspectorHelpers::DragScalar("##Points", &points, isDirty);
                TableNextField("Radius");
                EntityInspectorHelpers::DragFloatWithLimits("##ShapeRadius", &radius, 0.1f, 0.f, 100.f, isDirty);
                TableNextField("Outline Thickness");
                EntityInspectorHelpers::DragFloatWithLimits(
                    "##ShapeThickness", &outlineThickness, 0.1f, 0.f, 100.f, isDirty);
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
    j = { { "points", c.points },
        { "radius", c.radius },
        { "outlineThickness", c.outlineThickness },
        { "fillColor", { c.fillColor.r, c.fillColor.g, c.fillColor.b, c.fillColor.a } },
        { "outlineColor", { c.outlineColor.r, c.outlineColor.g, c.outlineColor.b, c.outlineColor.a } } };
}

inline void from_json(const Json& j, CShape& c)
{
    c.points = j.value("points", 3);
    c.radius = j.value("radius", 10.f);
    c.outlineThickness = j.value("outlineThickness", 1.f);

    auto fill = j.value("fillColor", std::vector<uint8_t> { 255, 255, 255, 255 });
    c.fillColor = sf::Color(fill[0], fill[1], fill[2], fill[3]);

    auto outline = j.value("outlineColor", std::vector<uint8_t> { 255, 255, 255, 255 });
    c.outlineColor = sf::Color(outline[0], outline[1], outline[2], outline[3]);
}

struct CCollider
{
    Vect2f size = Vect2f(32, 32);
    Vect2f halfSize = Vect2f(16, 16);
    Vect2f offset = Vect2f(0, 0);
    bool isTrigger = false;
    static constexpr const char* name = "Collider";

    CCollider() = default;
    CCollider(const Vect2f& sz, const Vect2f& off, bool trigger = false)
        : size(sz), halfSize(sz.x * 0.5f, sz.y * 0.5f), offset(off), isTrigger(trigger)
    {
    }
    REGISTER_COMPONENT(CCollider);

    void Reset()
    {
        size = Vect2f(32, 32);
        halfSize = Vect2f(16, 16);
        offset = Vect2f(0, 0);
        isTrigger = false;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CCollider>("Collider", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty))
        {
            if (ImGui::BeginTable("CColliderTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Size");
                EntityInspectorHelpers::DragFloat2(
                    "##ColliderWidth", &size.x, "##ColliderHeight", &size.y, 0.1f, isDirty);
                halfSize = { size.x * 0.5f, size.y * 0.5f };

                TableNextField("Offset");
                EntityInspectorHelpers::DragFloat2(
                    "##ColliderOffsetX", &offset.x, "##ColliderOffsetY", &offset.y, 0.1f, isDirty);
                TableNextField("Is Trigger");
                EntityInspectorHelpers::Checkbox("##ColliderTrigger", &isTrigger, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

inline void to_json(Json& j, const CCollider& c)
{
    j = { { "size", { { "x", c.size.x }, { "y", c.size.y } } },
        { "offset", { { "x", c.offset.x }, { "y", c.offset.y } } },
        { "isTrigger", c.isTrigger } };
}

inline void from_json(const Json& j, CCollider& c)
{
    c.size.x = j["size"].value("x", 1.f);
    c.size.y = j["size"].value("y", 1.f);
    c.offset.x = j["offset"].value("x", 0.f);
    c.offset.y = j["offset"].value("y", 0.f);
    c.isTrigger = j.value("isTrigger", false);
    c.halfSize = { c.size.x * 0.5f, c.size.y * 0.5f };
}

struct CRigidBody
{
    Vect2f velocity = Vect2f(0, 0);
    Vect2f previousPosition = Vect2f(0, 0);
    float mass = 1.f;
    float inverseMass = 1.f;
    float bounciness = 0.5f;
    bool isStatic = false;
    static constexpr const char* name = "RigidBody";

    CRigidBody() = default;
    CRigidBody(const Vect2f& vel, float m, float bounce, bool stat)
        : velocity(vel), mass(m), bounciness(bounce), isStatic(stat)
    {
        if (isStatic)
        {
            mass = 0;
            inverseMass = 0;
        }
        else
        {
            inverseMass = 1.0f / mass;
        }
    }
    REGISTER_COMPONENT(CRigidBody);

    void Reset()
    {
        velocity = Vect2f(0, 0);
        previousPosition = Vect2f(0, 0);
        mass = 1.f;
        inverseMass = 1.f;
        bounciness = 0.5f;
        isStatic = false;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CRigidBody>("RigidBody", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty))
        {
            if (ImGui::BeginTable("CRigidBodyTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Velocity");
                EntityInspectorHelpers::DragFloat2("##RBVelX", &velocity.x, "##RBVelY", &velocity.y, 0.1f, isDirty);
                TableNextField("Previous Position");
                EntityInspectorHelpers::DragFloat2(
                    "##PreviousPosX", &previousPosition.x, "##PreviousPosY", &previousPosition.y, 0.1f, isDirty);
                TableNextField("Mass");
                EntityInspectorHelpers::DragFloatWithLimits("##RBMass", &mass, 0.1f, 0.f, 10000.f, isDirty);
                TableNextField("Bounciness");
                EntityInspectorHelpers::DragFloatWithLimits("##RBBounciness", &bounciness, 0.01f, 0.f, 1.f, isDirty);
                TableNextField("Is Static");
                EntityInspectorHelpers::Checkbox("##RBStatic", &isStatic, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

inline void to_json(Json& j, const CRigidBody& c)
{
    j = { { "velocity", { { "x", c.velocity.x }, { "y", c.velocity.y } } },
        { "mass", c.mass },
        { "bounciness", c.bounciness },
        { "isStatic", c.isStatic } };
}

inline void from_json(const Json& j, CRigidBody& c)
{
    c.velocity.x = j["velocity"].value("x", 0.f);
    c.velocity.y = j["velocity"].value("y", 0.f);
    c.mass = j.value("mass", 1.f);
    c.bounciness = j.value("bounciness", 0.5f);
    c.isStatic = j.value("isStatic", false);

    if (c.isStatic)
    {
        c.mass = 0;
        c.inverseMass = 0.f;
    }
    else
    {
        c.inverseMass = 1.f / c.mass;
    }
}

struct CText
{
    std::string content = "";
    sf::Color textColor = sf::Color::White;
    Vect2f offset = Vect2f(0, 0);
    float fontSize = 12.f;
    static constexpr const char* name = "Text";

    CText() = default;
    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size)
        : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }
    REGISTER_COMPONENT(CText);

    void Reset()
    {
        content = "";
        textColor = sf::Color::White;
        offset = Vect2f(0, 0);
        fontSize = 12.f;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CText>("Text", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty))
        {
            if (ImGui::BeginTable("CTextTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Content");
                EntityInspectorHelpers::InputText("##TextContent", content, isDirty);
                TableNextField("Font Size");
                EntityInspectorHelpers::DragFloatWithLimits("##TextSize", &fontSize, 1.f, 1.f, 200.f, isDirty);
                TableNextField("Offset");
                EntityInspectorHelpers::DragFloat2(
                    "##TextOffsetX", &offset.x, "##TextOffsetY", &offset.y, 0.1f, isDirty);
                TableNextField("Text Color");
                EntityInspectorHelpers::ColorEdit4("##TextColor", textColor, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

inline void to_json(Json& j, const CText& c)
{
    j = { { "content", c.content },
        { "fontSize", c.fontSize },
        { "offset", { { "x", c.offset.x }, { "y", c.offset.y } } },
        { "textColor", { c.textColor.r, c.textColor.g, c.textColor.b, c.textColor.a } } };
}

inline void from_json(const Json& j, CText& c)
{
    c.content = j.value("content", std::string { "" });
    c.fontSize = j.value("fontSize", 12.f);
    c.offset.x = j["offset"].value("x", 0.f);
    c.offset.y = j["offset"].value("y", 0.f);

    auto col = j.value("textColor", std::vector<uint8_t> { 255, 255, 255, 255 });
    c.textColor = sf::Color(col[0], col[1], col[2], col[3]);
}

struct CNotDrawable
{
    static constexpr const char* name = "NotDrawable";

    CNotDrawable() = default;
    REGISTER_COMPONENT(CNotDrawable);

    void Reset(){};

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CNotDrawable>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CNotDrawable&) { j = Json::object(); }
inline void from_json(const Json&, CNotDrawable&) { }

struct CTower
{
    static constexpr const char* name = "Tower";

    CTower() = default;
    REGISTER_COMPONENT(CTower);

    void Reset(){};

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CTower>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CTower&) { j = Json::object(); }
inline void from_json(const Json&, CTower&) { }

struct CEnemy
{
    static constexpr const char* name = "Enemy";

    CEnemy() = default;
    REGISTER_COMPONENT(CEnemy);

    void Reset(){};

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CEnemy>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CEnemy&) { j = Json::object(); }
inline void from_json(const Json&, CEnemy&) { }
