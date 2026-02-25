#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Mouse.hpp>
#include <string>
#include "gui/EntityInspectorHelper.h"
#include "core/utils/Vect2.hpp"
#include "imgui.h"

using namespace EntityInspectorHelpers;

struct CTransform
{
    Vect2f position;
    Vect2f scale;
    float rotation;
    static constexpr const char* name = "Transfrom";

    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) { }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CTransform>(name, ptr);
        if (ImGui::BeginTable("CTransformTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Position");
            DragFloat2("##PosX", &position.x, "##PosY", &position.y);
            TableNextField("Scale");
            DragFloat2("##ScaleX", &scale.x, "##ScaleY", &scale.y);
            TableNextField("Rotation");
            DragFloatWithLimits("##Rotation", &rotation, 0.5f, -360.f, 360.f);
            ImGui::EndTable();
        }
    }
};

struct CMovement
{
    float speed;
    static constexpr const char* name = "Movement";
    CMovement(float spd) : speed(spd) { }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CMovement>(name, ptr);
        if (ImGui::BeginTable("CMovementTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Speed");
            DragFloatWithLimits("##MoveSpeed", &speed, 0.1f, 0.f, 10000.f);
            ImGui::EndTable();
        }
    }
};

struct CShape
{
    size_t points;
    sf::Color fillColor;
    sf::Color outlineColor;
    float radius;
    float outlineThickness;
    static constexpr const char* name = "Shape";

    CShape(size_t pts, const sf::Color& fill, const sf::Color& outline, float rad, float thickness)
        : points(pts), fillColor(fill), outlineColor(outline), radius(rad), outlineThickness(thickness)
    {
    }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CShape>(name, ptr);
        if (ImGui::BeginTable("CShapeTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Points");
            DragScalar("##Points", &points);
            TableNextField("Radius");
            DragFloatWithLimits("##ShapeRadius", &radius, 0.1f);
            TableNextField("Outline Thickness");
            DragFloatWithLimits("##ShapeThickness", &outlineThickness, 0.1f, 0.f, 100.f);
            TableNextField("Fill Color");
            ColorEdit4("##ShapeFill", fillColor);
            TableNextField("Outline Color");
            ColorEdit4("##ShapeOutline", outlineColor);
            ImGui::EndTable();
        }
    }
};

struct CCollider
{
    Vect2f size;
    Vect2f halfSize;
    Vect2f offset;
    bool isTrigger;
    static constexpr const char* name = "Collider";

    CCollider(const Vect2f& sz, const Vect2f& off, bool trigger = false)
        : size(sz), halfSize(sz.x * 0.5f, sz.y * 0.5f), offset(off), isTrigger(trigger)
    {
    }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CCollider>(name, ptr);

        if (ImGui::BeginTable("CColliderTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Size");
            DragFloat2("##ColliderWidth", &size.x, "##ColliderHeight", &size.y);
            halfSize = { size.x * 0.5f, size.y * 0.5f };

            TableNextField("Offset");
            DragFloat2("##ColliderOffsetX", &offset.x, "##ColliderOffsetY", &offset.y);
            TableNextField("Is Trigger");
            Checkbox("##ColliderTrigger", &isTrigger);

            ImGui::EndTable();
        }
    }
};

struct CRigidBody
{
    Vect2f velocity;
    Vect2f previousPosition;
    float mass;
    float inverseMass;
    float bounciness; // [0,1]
    bool isStatic;
    static constexpr const char* name = "RigidBody";

    CRigidBody(const Vect2f& vel, float m, float bounciness, bool stat) : velocity(vel), mass(m), bounciness(bounciness), isStatic(stat)
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

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CRigidBody>(name, ptr);

        if (ImGui::BeginTable("CRigidBodyTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Velocity");
            DragFloat2("##RBVelX", &velocity.x, "##RBVelY", &velocity.y);
            TableNextField("Previous Position");
            DragFloat2("##PreviousPosX", &previousPosition.x, "##PreviousPosY", &previousPosition.y);
            TableNextField("Mass");
            DragFloatWithLimits("##RBMass", &mass, 0.1f, 0.f, 10000.f);
            TableNextField("Bounciness");
            DragFloatWithLimits("##RBBounciness", &bounciness, 0.1f, 0.f, 1.f);
            TableNextField("Is Static");
            Checkbox("##RBStatic", &isStatic);
            ImGui::EndTable();
        }
    }
};

struct CText
{
    std::string content;
    sf::Color textColor;
    Vect2f offset;
    float fontSize;
    static constexpr const char* name = "Text";

    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size) : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CText>(name, ptr);

        if (ImGui::BeginTable("CTextTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Content");
            InputText("##TextContent", content);
            TableNextField("Font Size");
            DragFloatWithLimits("##TextSize", &fontSize, 1.f, 1.f, 200.f);
            TableNextField("Offset");
            DragFloat2("##TextOffsetX", &offset.x, "##TextOffsetY", &offset.y);
            TableNextField("Text Color");
            ColorEdit4("##TextColor", textColor);

            ImGui::EndTable();
        }
    }
};

struct CNotDrawable
{
    static constexpr const char* name = "NotDrawable";
    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CNotDrawable>(name, ptr);

        if (ImGui::BeginTable("CNotDrawableTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Info");
            ImGui::TextDisabled("This entity is not renderable.");
            ImGui::EndTable();
        }
    }
};
