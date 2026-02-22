#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cstdint>
#include <format>
#include <SFML/Graphics/CircleShape.hpp>
#include <string>
#include "core/utils/Vect2.hpp"
#include "imgui.h"

struct CTransform
{
    Vect2f position;
    Vect2f scale;
    float rotation;

    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) { }

    std::string GetDescription() const
    {
        return std::format("position ({},{}) rotation ({}) scale ({},{})", position.x, position.y, rotation, scale.x, scale.y);
    }

    void GuiInspectorDisplay()
    {
        ImGui::SeparatorText("Transform");

        ImGui::Text("Position");
        ImGui::PushItemWidth(150.f);
        ImGui::DragFloat("##TransformPosX", &position.x, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("##TransformPosY", &position.y, 0.1f);

        ImGui::Spacing();

        ImGui::Text("Scale");
        ImGui::DragFloat("##TransformScaleX", &scale.x, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("##TransformScaleY", &scale.y, 0.1f);

        ImGui::Spacing();

        ImGui::Text("Rotation");
        ImGui::DragFloat("##TransformRotation", &rotation, 0.5f, -360.f, 360.f);
        ImGui::PopItemWidth();
    }
};

// ============================================================
// MOVEMENT
// ============================================================

struct CMovement
{
    Vect2f velocity;
    float speed;

    CMovement(const Vect2f& vel, float spd) : velocity(vel), speed(spd) { }

    std::string GetDescription() const { return std::format("Velocity:({},{}) Speed:{}", velocity.x, velocity.y, speed); }

    void GuiInspectorDisplay()
    {
        ImGui::SeparatorText("Movement");

        ImGui::Text("Velocity");
        ImGui::DragFloat("X##MoveVelX", &velocity.x, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("Y##MoveVelY", &velocity.y, 0.1f);

        ImGui::Spacing();

        ImGui::DragFloat("Speed##MoveSpeed", &speed, 0.1f, 0.0f, 10000.0f);
    }
};

// ============================================================
// SHAPE
// ============================================================

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

    void GuiInspectorDisplay()
    {
        ImGui::SeparatorText("Shape");

        ImGui::InputScalar("Points##ShapePoints", ImGuiDataType_U64, &points);

        ImGui::DragFloat("Radius##ShapeRadius", &radius, 0.1f, 0.0f, 10000.0f);

        ImGui::DragFloat("Outline Thickness##ShapeThickness", &outlineThickness, 0.1f, 0.0f, 100.0f);

        float fill[4] = { fillColor.r / 255.f, fillColor.g / 255.f, fillColor.b / 255.f, fillColor.a / 255.f };

        if (ImGui::ColorEdit4("Fill Color##ShapeFill", fill))
        {
            fillColor = sf::Color(static_cast<uint8_t>(fill[0] * 255),
                static_cast<uint8_t>(fill[1] * 255),
                static_cast<uint8_t>(fill[2] * 255),
                static_cast<uint8_t>(fill[3] * 255));
        }

        float outline[4] = { outlineColor.r / 255.f, outlineColor.g / 255.f, outlineColor.b / 255.f, outlineColor.a / 255.f };

        if (ImGui::ColorEdit4("Outline Color##ShapeOutline", outline))
        {
            outlineColor = sf::Color(static_cast<uint8_t>(outline[0] * 255),
                static_cast<uint8_t>(outline[1] * 255),
                static_cast<uint8_t>(outline[2] * 255),
                static_cast<uint8_t>(outline[3] * 255));
        }
    }
};

// ============================================================
// COLLIDER
// ============================================================

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
        return std::format("Size: ({}, {}) , offset: ({}, {}) , isTrigger: {}", size.x, size.y, offset.x, offset.y, isTrigger);
    }

    void GuiInspectorDisplay()
    {
        ImGui::SeparatorText("Collider");

        ImGui::Text("Size");
        ImGui::DragFloat("W##ColliderWidth", &size.x, 0.1f, 0.0f);
        ImGui::SameLine();
        ImGui::DragFloat("H##ColliderHeight", &size.y, 0.1f, 0.0f);

        halfSize = { size.x * 0.5f, size.y * 0.5f };

        ImGui::Spacing();

        ImGui::Text("Offset");
        ImGui::DragFloat("X##ColliderOffsetX", &offset.x, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("Y##ColliderOffsetY", &offset.y, 0.1f);

        ImGui::Spacing();

        ImGui::Checkbox("Is Trigger##ColliderTrigger", &isTrigger);
    }
};

// ============================================================
// RIGIDBODY
// ============================================================

struct CRigidBody
{
    Vect2f velocity;
    float mass;
    bool isStatic;

    CRigidBody(const Vect2f& vel, float m, bool stat) : velocity(vel), mass(m), isStatic(stat) { }

    std::string GetDescription() const { return std::format("Velocity:({},{}) Mass:{} Static:{}", velocity.x, velocity.y, mass, isStatic); }

    void GuiInspectorDisplay()
    {
        ImGui::SeparatorText("RigidBody");

        ImGui::Text("Velocity");
        ImGui::DragFloat("X##RBVelX", &velocity.x, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("Y##RBVelY", &velocity.y, 0.1f);

        ImGui::Spacing();

        ImGui::DragFloat("Mass##RBMass", &mass, 0.1f, 0.0f, 10000.0f);

        ImGui::Checkbox("Is Static##RBStatic", &isStatic);

        if (!isStatic && mass > 0.f)
        {
            ImGui::Text("Inverse Mass: %.4f", 1.f / mass);
        }
        else
        {
            ImGui::Text("Inverse Mass: 0 (Static)");
        }
    }
};

// ============================================================
// TEXT
// ============================================================

struct CText
{
    std::string content;
    sf::Color textColor;
    Vect2f offset;
    float fontSize;

    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size) : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }

    std::string GetDescription() const { return content; }

    void GuiInspectorDisplay()
    {
        ImGui::SeparatorText("Text");

        char buffer[256] {};
        strncpy(buffer, content.c_str(), sizeof(buffer) - 1);

        if (ImGui::InputText("Content##TextContent", buffer, sizeof(buffer)))
        {
            content = buffer;
        }

        ImGui::DragFloat("Font Size##TextSize", &fontSize, 1.0f, 1.0f, 200.0f);

        ImGui::Spacing();

        ImGui::Text("Offset");
        ImGui::DragFloat("X##TextOffsetX", &offset.x, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("Y##TextOffsetY", &offset.y, 0.1f);

        float color[4] = { textColor.r / 255.f, textColor.g / 255.f, textColor.b / 255.f, textColor.a / 255.f };

        if (ImGui::ColorEdit4("Text Color##TextColor", color))
        {
            textColor = sf::Color(static_cast<uint8_t>(color[0] * 255),
                static_cast<uint8_t>(color[1] * 255),
                static_cast<uint8_t>(color[2] * 255),
                static_cast<uint8_t>(color[3] * 255));
        }
    }
};

// ============================================================
// NOT DRAWABLE
// ============================================================

struct CNotDrawable
{
    std::string GetDescription() const { return ""; }

    void GuiInspectorDisplay()
    {
        ImGui::SeparatorText("Not Drawable");
        ImGui::TextDisabled("This entity is not renderable.");
    }
};
