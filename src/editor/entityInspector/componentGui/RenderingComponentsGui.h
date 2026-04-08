#pragma once

#include "editor/entityInspector/componentGui/ComponentGui.h"
#include "rendering/RenderingComponents.hpp"

template <>
struct ComponentInspectorGui<CSprite>
{
    static void Display(CSprite& sprite, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CSprite>("Sprite", &sprite, RemoveComponentCallback, [&] { sprite = CSprite{}; }, isDirty))
        {
            if (ImGui::BeginTable("CSpriteTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Texture");
                EntityInspectorHelpers::InputText("##TextureName", sprite.textureName, isDirty);
                TableNextField("Size");
                EntityInspectorHelpers::DragFloat2("##SpriteSizeX", &sprite.size.x, "##SpriteSizeY", &sprite.size.y, 0.1f, isDirty);
                TableNextField("Color");
                EntityInspectorHelpers::ColorEdit4("##SpriteColor", sprite.color, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct ComponentInspectorGui<CShape>
{
    static void Display(CShape& shape, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CShape>("Shape", &shape, RemoveComponentCallback, [&] { shape = CShape{}; }, isDirty))
        {
            if (ImGui::BeginTable("CShapeTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Points");
                EntityInspectorHelpers::DragScalar("##Points", &shape.points, isDirty);
                TableNextField("Radius");
                EntityInspectorHelpers::DragFloatWithLimits("##ShapeRadius", &shape.radius, 0.1f, 0.f, 100.f, isDirty);
                TableNextField("Outline Thickness");
                EntityInspectorHelpers::DragFloatWithLimits("##ShapeThickness", &shape.outlineThickness, 0.1f, 0.f, 100.f, isDirty);
                TableNextField("Fill Color");
                EntityInspectorHelpers::ColorEdit4("##ShapeFill", shape.fillColor, isDirty);
                TableNextField("Outline Color");
                EntityInspectorHelpers::ColorEdit4("##ShapeOutline", shape.outlineColor, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct ComponentInspectorGui<CText>
{
    static void Display(CText& text, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CText>("Text", &text, RemoveComponentCallback, [&] { text = CText{}; }, isDirty))
        {
            if (ImGui::BeginTable("CTextTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Content");
                EntityInspectorHelpers::InputText("##TextContent", text.content, isDirty);
                TableNextField("Font Size");
                EntityInspectorHelpers::DragFloatWithLimits("##TextSize", &text.fontSize, 1.f, 1.f, 200.f, isDirty);
                TableNextField("Offset");
                EntityInspectorHelpers::DragFloat2("##TextOffsetX", &text.offset.x, "##TextOffsetY", &text.offset.y, 0.1f, isDirty);
                TableNextField("Text Color");
                EntityInspectorHelpers::ColorEdit4("##TextColor", text.textColor, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct ComponentInspectorGui<CNotDrawable>
{
    static void Display(CNotDrawable& c, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CNotDrawable>("NotDrawable", &c, RemoveComponentCallback, [&] { c = CNotDrawable{}; }, isDirty);
    }
};
