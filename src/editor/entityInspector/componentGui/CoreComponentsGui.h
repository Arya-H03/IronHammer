#pragma once

#include "core/CoreComponents.hpp"
#include "editor/entityInspector/EntityInspectorHelper.h"
#include "editor/entityInspector/componentGui/ComponentGui.h"

template <>
struct ComponentInspectorGui<CTransform>
{
    static void Display(CTransform& transform, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CTransform>("Transform", &transform, RemoveComponentCallback, [&] { transform = CTransform{}; }, isDirty))
        {
            if (ImGui::BeginTable("CTransformTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Position");
                EntityInspectorHelpers::DragFloat2("##PosX", &transform.position.x, "##PosY", &transform.position.y, 0.1f, isDirty);
                if(isDirty != nullptr && *isDirty)
                {
                    transform.previousPosition = transform.position;
                }

                TableNextField("Previous Position");
                EntityInspectorHelpers::DragFloat2("##PrevPosX", &transform.previousPosition.x, "##PrevPosY", &transform.previousPosition.y, 0.1f, isDirty);
                TableNextField("Scale");
                EntityInspectorHelpers::DragFloat2("##ScaleX", &transform.scale.x, "##ScaleY", &transform.scale.y, 0.1f, isDirty);
                TableNextField("Rotation");
                EntityInspectorHelpers::DragFloatWithLimits("##Rotation", &transform.rotation, 0.5f, -360.f, 360.f, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct ComponentInspectorGui<CMovement>
{
    static void Display(CMovement& movement, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CMovement>("Movement", &movement, RemoveComponentCallback, [&] { movement = CMovement{}; }, isDirty))
        {
            if (ImGui::BeginTable("CMovementTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Speed");
                EntityInspectorHelpers::DragFloatWithLimits("##MoveSpeed", &movement.speed, 0.1f, 0.f, 10000.f, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct ComponentInspectorGui<CMolded>
{
    static void Display(CMolded& moldDerived, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CMolded>(CMolded::name, &moldDerived, isDirty))
        {
            if (ImGui::BeginTable("MoldDerivedTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Mold");
                EntityInspectorHelpers::TextBox(moldDerived.moldName.c_str());
                ImGui::EndTable();
            }
        }
    }
};
