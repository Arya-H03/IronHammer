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

    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) { }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CTransform>("CTransform", ptr);
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
    Vect2f velocity;
    float speed;

    CMovement(const Vect2f& vel, float spd) : velocity(vel), speed(spd) { }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CMovement>("CMovement", ptr);
        if (ImGui::BeginTable("CMovementTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Velocity");
            DragFloat2("##VelX", &velocity.x, "##VelY", &velocity.y);
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

    CShape(size_t pts, const sf::Color& fill, const sf::Color& outline, float rad, float thickness)
        : points(pts), fillColor(fill), outlineColor(outline), radius(rad), outlineThickness(thickness)
    {
    }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CShape>("CShape", ptr);
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

    CCollider(const Vect2f& sz, const Vect2f& off, bool trigger = false)
        : size(sz), halfSize(sz.x * 0.5f, sz.y * 0.5f), offset(off), isTrigger(trigger)
    {
    }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CCollider>("CCollider", ptr);

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
    float mass;
    bool isStatic;

    CRigidBody(const Vect2f& vel, float m, bool stat) : velocity(vel), mass(m), isStatic(stat) { }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CRigidBody>("CRigidBody", ptr);

        if (ImGui::BeginTable("CRigidBodyTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Velocity");
            DragFloat2("##RBVelX", &velocity.x, "##RBVelY", &velocity.y);
            TableNextField("Mass");
            DragFloatWithLimits("##RBMass", &mass, 0.1f, 0.f, 10000.f);
            TableNextField("Is Static");
            Checkbox("##RBStatic", &isStatic);

            TableNextField("Inverse Mass");
            if (!isStatic && mass > 0.f)
                ImGui::Text("%.4f", 1.f / mass);
            else
                ImGui::Text("0 (Static)");

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

    CText(const std::string& txt, const sf::Color& color, const Vect2f& off, float size) : content(txt), textColor(color), offset(off), fontSize(size)
    {
    }

    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CText>("CText", ptr);

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
    void GuiInspectorDisplay(void* ptr)
    {
        TypeHeader<CNotDrawable>("CNotDrawable", ptr);

        if (ImGui::BeginTable("CNotDrawableTable", 2, ImGuiTableFlags_SizingFixedFit))
        {
            TableNextField("Info");
            ImGui::TextDisabled("This entity is not renderable.");
            ImGui::EndTable();
        }
    }
};
