#pragma once

#include "editor/entityInspector/componentGui/ComponentGui.h"
#include "physics/PhysicsComponents.hpp"

template <>
struct ComponentInspectorGui<CCollider>
{
    static void Display(CCollider& collider, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CCollider>("Collider", &collider, RemoveComponentCallback, [&] { collider = CCollider{}; }, isDirty))
        {
            if (ImGui::BeginTable("CColliderTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Size");
                EntityInspectorHelpers::DragFloat2("##ColliderWidth", &collider.size.x, "##ColliderHeight", &collider.size.y, 0.1f,
                                                   isDirty);
                collider.halfSize = {collider.size.x * 0.5f, collider.size.y * 0.5f};

                TableNextField("Offset");
                EntityInspectorHelpers::DragFloat2("##ColliderOffsetX", &collider.offset.x, "##ColliderOffsetY", &collider.offset.y, 0.1f,
                                                   isDirty);

                TableNextField("Layer");
                const char* currentLayerName = "Unknown";
                for (size_t i = 0; i < layerCount; i++)
                {
                    if (layerValues[i] == collider.layer)
                    {
                        currentLayerName = layerNames[i];
                        break;
                    }
                }
                EntityInspectorHelpers::ComboBoxSingleSelect(
                    "##LayerComboBox", currentLayerName, layerNames, layerCount,
                    [&collider](size_t selectedIndex) { collider.layer = layerValues[selectedIndex]; }, isDirty);

                TableNextField("Mask");
                size_t selectedCount = 0;
                const char* selectedOptionName = "None";
                bool selectedOptions[layerCount];
                for (size_t i = 0; i < layerCount; i++)
                {
                    if ((collider.mask & static_cast<uint32_t>(layerValues[i])) != 0)
                    {
                        selectedOptions[i] = true;
                        selectedCount++;
                        selectedOptionName = layerNames[i];
                    }
                    else
                    {
                        selectedOptions[i] = false;
                    }
                }
                if (selectedCount >= 2) selectedOptionName = "Multiple";
                EntityInspectorHelpers::ComboBoxMultipleSelect(
                    "##MaskComboBox", selectedOptionName, layerNames, layerCount, selectedOptions,
                    [&collider](size_t selectedIndex)
                    {
                        if ((collider.mask & static_cast<uint32_t>(layerValues[selectedIndex])) == 0)
                            collider.mask |= static_cast<uint32_t>(layerValues[selectedIndex]);
                        else
                            collider.mask &= ~static_cast<uint32_t>(layerValues[selectedIndex]);
                    },
                    isDirty);

                TableNextField("Is Trigger");
                EntityInspectorHelpers::Checkbox("##ColliderTrigger", &collider.isTrigger, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct ComponentInspectorGui<CRigidBody>
{
    static void Display(CRigidBody& rb, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CRigidBody>("RigidBody", &rb, RemoveComponentCallback, [&] { rb = CRigidBody{}; }, isDirty))
        {
            if (ImGui::BeginTable("CRigidBodyTable", 2, ImGuiTableFlags_SizingFixedFit))
            {
                TableNextField("Velocity");
                EntityInspectorHelpers::DragFloat2("##RBVelX", &rb.velocity.x, "##RBVelY", &rb.velocity.y, 0.1f, isDirty);
                TableNextField("Acceleration");
                EntityInspectorHelpers::DragFloat2("##RBAccelX", &rb.acceleration.x, "##RBAccelY", &rb.acceleration.y, 0.1f, isDirty);
                TableNextField("Previous Position");
                EntityInspectorHelpers::DragFloat2("##PreviousPosX", &rb.previousPosition.x, "##PreviousPosY", &rb.previousPosition.y, 0.1f,
                                                   isDirty);
                TableNextField("Mass");
                EntityInspectorHelpers::DragFloatWithLimits("##RBMass", &rb.mass, 0.1f, 0.f, 10000.f, isDirty);
                TableNextField("Bounciness");
                EntityInspectorHelpers::DragFloatWithLimits("##RBBounciness", &rb.bounciness, 0.01f, 0.f, 1.f, isDirty);
                TableNextField("Is Static");
                EntityInspectorHelpers::Checkbox("##RBStatic", &rb.isStatic, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

//Fix me: might want to diplay all the entities that are in the events
template <>
struct ComponentInspectorGui<CCollisionEvent>
{
    static void Display(CCollisionEvent& collisionEvent, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CCollisionEvent>(CCollisionEvent::name, &collisionEvent, RemoveComponentCallback, [&] { collisionEvent = CCollisionEvent{}; }, isDirty);
    }
};

template <>
struct ComponentInspectorGui<CCollisionEnter>
{
    static void Display(CCollisionEnter& c, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CCollisionEnter>("CollisionEnter", &c, RemoveComponentCallback, [&] { c = CCollisionEnter{}; }, isDirty);
    }
};

template <>
struct ComponentInspectorGui<CCollisionExit>
{
    static void Display(CCollisionExit& c, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CCollisionExit>("CollisionExit", &c, RemoveComponentCallback, [&] { c = CCollisionExit{}; }, isDirty);
    }
};

template <>
struct ComponentInspectorGui<CCollisionStay>
{
    static void Display(CCollisionStay& c, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CCollisionStay>("CollisionStay", &c, RemoveComponentCallback, [&] { c = CCollisionStay{}; }, isDirty);
    }
};
