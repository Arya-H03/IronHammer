#pragma once
#include "core/reflection/ComponentReflection.h"
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"
#include "imgui.h"

#include <nlohmann/json.hpp>
#include <tuple>

using namespace EntityInspectorHelpers;

struct CTransform
{
    Vect2f position = Vect2f(0, 0);
    Vect2f scale    = Vect2f(1, 1);
    float  rotation = 0.f;

    static constexpr const char* name = "Transform";

    CTransform() = default;
    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) {}
    REGISTER_COMPONENT(CTransform);

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CTransform>(
                "Transform", ptr, RemoveComponentCallback, [this] { *this = CTransform{}; }, isDirty)) {
            if (ImGui::BeginTable("CTransformTable", 2, ImGuiTableFlags_SizingFixedFit)) {
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

template <>
struct Reflect<CTransform>
{
    static constexpr auto fields =
        std::make_tuple(std::pair{"position", &CTransform::position}, std::pair{"scale", &CTransform::scale},
                        std::pair{"rotation", &CTransform::rotation});
};

struct CMovement
{
    float speed = 0.f;

    static constexpr const char* name = "Movement";

    CMovement() = default;
    CMovement(float spd) : speed(spd) {}
    REGISTER_COMPONENT(CMovement);

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CMovement>("Movement", ptr, RemoveComponentCallback, [this] { *this = CMovement{}; }, isDirty)) {
            if (ImGui::BeginTable("CMovementTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                TableNextField("Speed");
                EntityInspectorHelpers::DragFloatWithLimits("##MoveSpeed", &speed, 0.1f, 0.f, 10000.f, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

template <>
struct Reflect<CMovement>
{
    static constexpr auto fields = std::make_tuple(std::pair{"speed", &CMovement::speed});
};
