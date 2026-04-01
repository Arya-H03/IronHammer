#pragma once
#include "core/utils/Vect2.hpp"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"
#include "imgui.h"

#include <nlohmann/json.hpp>

using namespace EntityInspectorHelpers;
using Json = nlohmann::json;

struct CTransform
{
    Vect2f                       position = Vect2f(0, 0);
    Vect2f                       scale    = Vect2f(1, 1);
    float                        rotation = 0.f;
    static constexpr const char* name     = "Transform";

    CTransform() = default;
    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) {}
    REGISTER_COMPONENT(CTransform);

    void Reset()
    {
        position = Vect2f(0, 0);
        scale    = Vect2f(1, 1);
        rotation = 0.f;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CTransform>("Transform", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty)) {
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

inline void to_json(Json& json, const CTransform& c)
{
    json = {{"position", {{"x", c.position.x}, {"y", c.position.y}}},
            {"scale", {{"x", c.scale.x}, {"y", c.scale.y}}},
            {"rotation", c.rotation}};
}

inline void from_json(const Json& json, CTransform& c)
{
    c.position.x = json["position"].value("x", 0.f);
    c.position.y = json["position"].value("y", 0.f);
    c.scale.x    = json["scale"].value("x", 1.f);
    c.scale.y    = json["scale"].value("y", 1.f);
    c.rotation   = json["rotation"];
}

struct CMovement
{
    float                        speed = 0.f;
    static constexpr const char* name  = "Movement";

    CMovement() = default;
    CMovement(float spd) : speed(spd) {}
    REGISTER_COMPONENT(CMovement);

    void Reset() { speed = 0.f; }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CMovement>("Movement", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty)) {
            if (ImGui::BeginTable("CMovementTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                TableNextField("Speed");
                EntityInspectorHelpers::DragFloatWithLimits("##MoveSpeed", &speed, 0.1f, 0.f, 10000.f, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

inline void to_json(Json& j, const CMovement& c) { j = {{"speed", c.speed}}; }
inline void from_json(const Json& j, CMovement& c) { c.speed = j.value("speed", 0.f); }
