#pragma once

#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"

using namespace EntityInspectorHelpers;
using Json = nlohmann::json;

enum class Layer : uint32_t {

    Default = 1 << 0,
    Tower   = 1 << 1,
    Enemy   = 1 << 2,
    Bullet  = 1 << 3
};

static constexpr Layer       layerValues[] = {Layer::Default, Layer::Tower, Layer::Enemy, Layer::Bullet};
static constexpr const char* layerNames[]  = {"Default", "Tower", "Enemy", "Bullet"};
static constexpr size_t      layerCount    = sizeof(layerValues) / sizeof(layerValues[0]);

struct CCollider
{
    Vect2f size     = Vect2f(32, 32);
    Vect2f halfSize = Vect2f(16, 16);
    Vect2f offset   = Vect2f(0, 0);

    Layer    layer = Layer::Default;
    uint32_t mask  = ~0u;

    bool isTrigger = false;

    static constexpr const char* name = "Collider";

    CCollider() = default;
    CCollider(const Vect2f& sz, const Vect2f& off, Layer layer = Layer::Default, uint32_t mask = ~0u, bool trigger = false)
        : size(sz), halfSize(sz.x * 0.5f, sz.y * 0.5f), offset(off), layer(layer), mask(mask), isTrigger(trigger)
    {
    }
    REGISTER_COMPONENT(CCollider);

    void Reset()
    {
        size      = Vect2f(32, 32);
        halfSize  = Vect2f(16, 16);
        offset    = Vect2f(0, 0);
        layer     = Layer::Default;
        mask      = ~0u;
        isTrigger = false;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CCollider>("Collider", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty)) {
            if (ImGui::BeginTable("CColliderTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                TableNextField("Size");
                EntityInspectorHelpers::DragFloat2("##ColliderWidth", &size.x, "##ColliderHeight", &size.y, 0.1f, isDirty);
                halfSize = {size.x * 0.5f, size.y * 0.5f};

                TableNextField("Offset");
                EntityInspectorHelpers::DragFloat2("##ColliderOffsetX", &offset.x, "##ColliderOffsetY", &offset.y, 0.1f,
                                                   isDirty);
                TableNextField("Layer");

                const char* currentLayerName = "Unknown";
                for (size_t i = 0; i < layerCount; i++) {
                    if (layerValues[i] == layer) {
                        currentLayerName = layerNames[i];
                        break;
                    }
                }

                EntityInspectorHelpers::ComboBoxSingleSelect(
                    "##LayerComboBox", currentLayerName, layerNames, layerCount,
                    [this](size_t selectedIndex) { layer = layerValues[selectedIndex]; }, isDirty);

                TableNextField("Mask");

                size_t      selectedCount      = 0;
                const char* selectedOptionName = "None";
                bool        selectedOptions[layerCount];
                for (size_t i = 0; i < layerCount; i++) {
                    if ((mask & static_cast<uint32_t>(layerValues[i])) != 0) {
                        selectedOptions[i] = true;
                        selectedCount++;
                        selectedOptionName = layerNames[i];
                    }
                    else {
                        selectedOptions[i] = false;
                    }
                }

                if (selectedCount >= 2) selectedOptionName = "Multiple";

                EntityInspectorHelpers::ComboBoxMultipleSelect(
                    "##MasKComboBox", selectedOptionName, layerNames, layerCount, selectedOptions,
                    [this](size_t selectedIndex) {
                        if ((mask & static_cast<uint32_t>(layerValues[selectedIndex])) == 0) {
                            mask |= static_cast<uint32_t>(layerValues[selectedIndex]);
                        }
                        else {
                            mask &= ~static_cast<uint32_t>(layerValues[selectedIndex]);
                        }
                    },
                    isDirty);

                TableNextField("Is Trigger");
                EntityInspectorHelpers::Checkbox("##ColliderTrigger", &isTrigger, isDirty);
                ImGui::EndTable();
            }
        }
    }
};

inline void to_json(Json& j, const CCollider& c)
{
    j = {{"size", {{"x", c.size.x}, {"y", c.size.y}}},
         {"offset", {{"x", c.offset.x}, {"y", c.offset.y}}},
         {"layer", static_cast<uint32_t>(c.layer)},
         {"mask", c.mask},
         {"isTrigger", c.isTrigger}};
}

inline void from_json(const Json& j, CCollider& c)
{
    c.size.x    = j["size"].value("x", 1.f);
    c.size.y    = j["size"].value("y", 1.f);
    c.offset.x  = j["offset"].value("x", 0.f);
    c.offset.y  = j["offset"].value("y", 0.f);
    c.layer     = static_cast<Layer>(j.value("layer", static_cast<uint32_t>(Layer::Default)));
    c.mask      = j.value("mask", ~0u);
    c.isTrigger = j.value("isTrigger", false);
    c.halfSize  = {c.size.x * 0.5f, c.size.y * 0.5f};
}

struct CRigidBody
{
    Vect2f velocity         = Vect2f(0, 0);
    Vect2f previousPosition = Vect2f(0, 0);
    float  mass             = 1.f;
    float  inverseMass      = 1.f;
    float  bounciness       = 0.5f;
    bool   isStatic         = false;

    static constexpr const char* name = "RigidBody";

    CRigidBody() = default;
    CRigidBody(const Vect2f& vel, float m, float bounce, bool stat)
        : velocity(vel), mass(m), bounciness(bounce), isStatic(stat)
    {
        if (isStatic) {
            mass        = 0;
            inverseMass = 0;
        }
        else {
            inverseMass = 1.0f / mass;
        }
    }
    REGISTER_COMPONENT(CRigidBody);

    void Reset()
    {
        velocity         = Vect2f(0, 0);
        previousPosition = Vect2f(0, 0);
        mass             = 1.f;
        inverseMass      = 1.f;
        bounciness       = 0.5f;
        isStatic         = false;
    }

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        if (ComponentHeader<CRigidBody>("RigidBody", ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty)) {
            if (ImGui::BeginTable("CRigidBodyTable", 2, ImGuiTableFlags_SizingFixedFit)) {
                TableNextField("Velocity");
                EntityInspectorHelpers::DragFloat2("##RBVelX", &velocity.x, "##RBVelY", &velocity.y, 0.1f, isDirty);
                TableNextField("Previous Position");
                EntityInspectorHelpers::DragFloat2("##PreviousPosX", &previousPosition.x, "##PreviousPosY",
                                                   &previousPosition.y, 0.1f, isDirty);
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
    j = {{"velocity", {{"x", c.velocity.x}, {"y", c.velocity.y}}},
         {"mass", c.mass},
         {"bounciness", c.bounciness},
         {"isStatic", c.isStatic}};
}

inline void from_json(const Json& j, CRigidBody& c)
{
    c.velocity.x = j["velocity"].value("x", 0.f);
    c.velocity.y = j["velocity"].value("y", 0.f);
    c.mass       = j.value("mass", 1.f);
    c.bounciness = j.value("bounciness", 0.5f);
    c.isStatic   = j.value("isStatic", false);

    if (c.isStatic) {
        c.mass        = 0;
        c.inverseMass = 0.f;
    }
    else {
        c.inverseMass = 1.f / c.mass;
    }
}

struct CCollisionEnter
{
    Entity entity1;
    Entity entity2;

    static constexpr const char* name = "CollisionEnter";

    CCollisionEnter() = default;
    CCollisionEnter(Entity a, Entity b) : entity1(a), entity2(b) {}

    REGISTER_COMPONENT(CCollisionEnter);

    void Reset()
    {
        entity1 = {};
        entity2 = {};
    };

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CCollisionEnter>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CCollisionEnter&) { j = Json::object(); }
inline void from_json(const Json&, CCollisionEnter&) {}

struct CCollisionExit
{
    Entity entity1;
    Entity entity2;

    static constexpr const char* name = "CollisionExit";

    CCollisionExit() = default;
    CCollisionExit(Entity a, Entity b) : entity1(a), entity2(b) {}

    REGISTER_COMPONENT(CCollisionExit);

    void Reset()
    {
        entity1 = {};
        entity2 = {};
    };

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CCollisionExit>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CCollisionExit&) { j = Json::object(); }
inline void from_json(const Json&, CCollisionExit&) {}

struct CCollisionStay
{
    Entity entity1;
    Entity entity2;

    static constexpr const char* name = "CollisionStay";

    CCollisionStay() = default;
    CCollisionStay(Entity a, Entity b) : entity1(a), entity2(b) {}

    REGISTER_COMPONENT(CCollisionStay);

    void Reset()
    {
        entity1 = {};
        entity2 = {};
    };

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CCollisionStay>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CCollisionStay&) { j = Json::object(); }
inline void from_json(const Json&, CCollisionStay&) {}
