#pragma once

#include "core/reflection/ComponentReflection.h"
#include "core/utils/Vect2.hpp"
#include "ecs/common/ECSCommon.h"

#include <cstdint>

using Json = nlohmann::json;

enum class Layer : uint32_t
{

    Default = 1 << 0,
    Tower = 1 << 1,
    Enemy = 1 << 2,
    Bullet = 1 << 3
};

static constexpr Layer layerValues[] = {Layer::Default, Layer::Tower, Layer::Enemy, Layer::Bullet};
static constexpr const char* layerNames[] = {"Default", "Tower", "Enemy", "Bullet"};
static constexpr size_t layerCount = sizeof(layerValues) / sizeof(layerValues[0]);

inline void to_json(Json& json, const Layer& layer)
{
    json = static_cast<uint32_t>(layer);
}
inline void from_json(const Json& json, Layer& layer)
{
    layer = static_cast<Layer>(json.get<uint32_t>());
}

struct CCollider
{

    static constexpr const char* name = "Collider";

    Vect2f size = Vect2f(32, 32);
    Vect2f halfSize = Vect2f(16, 16);
    Vect2f offset = Vect2f(0, 0);
    Layer layer = Layer::Default;
    uint32_t mask = ~0u;
    bool isTrigger = false;

    CCollider() = default;
    CCollider(const Vect2f& sz, const Vect2f& off, Layer layer = Layer::Default, uint32_t mask = ~0u, bool trigger = false)
        : size(sz), halfSize(sz.x * 0.5f, sz.y * 0.5f), offset(off), layer(layer), mask(mask), isTrigger(trigger)
    {
    }

    void OnAfterDeserialize()
    {
        halfSize = Vect2f{size.x * 0.5f, size.y * 0.5f};
    }
};

template <>
struct Reflect<CCollider>
{
    static constexpr auto fields =
        std::make_tuple(Descriptor{"size", &CCollider::size, true}, Descriptor{"offset", &CCollider::offset, true},
                        Descriptor{"layer", &CCollider::layer, true}, Descriptor{"mask", &CCollider::mask, true},
                        Descriptor{"isTrigger", &CCollider::isTrigger, true});
};

struct CRigidBody
{
    static constexpr const char* name = "RigidBody";

    Vect2f velocity = Vect2f{0, 0};
    Vect2f previousPosition = Vect2f{0, 0};
    Vect2f acceleration = Vect2f{0, 0};
    float mass = 1.f;
    float inverseMass = 1.f;
    float bounciness = 0.5f;
    bool isStatic = false;

    CRigidBody() = default;
    CRigidBody(const Vect2f& vel, const Vect2f& accel, float m, float bounce, bool stat)
        : velocity(vel), mass(m), bounciness(bounce), acceleration(accel), isStatic(stat)
    {
        if (isStatic)
        {
            mass = 0;
            inverseMass = 0;
        }
        else
            inverseMass = 1.0f / mass;
    }

    void OnAfterDeserialize()
    {
        if (isStatic)
        {
            mass = 0;
            inverseMass = 0.f;
        }
        else
            inverseMass = 1.f / mass;
    }
};

template <>
struct Reflect<CRigidBody>
{
    static constexpr auto fields =
        std::make_tuple(Descriptor{"velocity", &CRigidBody::velocity, true}, Descriptor{"mass", &CRigidBody::mass, true},
                        Descriptor{"bounciness", &CRigidBody::bounciness, true},
                        Descriptor{"acceleration", &CRigidBody::acceleration, true}, Descriptor{"isStatic", &CRigidBody::isStatic, true});
};

struct CCollisionEnter
{
    static constexpr const char* name = "CollisionEnter";

    Entity entity1;
    Entity entity2;

    CCollisionEnter() = default;
    CCollisionEnter(Entity a, Entity b) : entity1(a), entity2(b)
    {
    }
};

template <>
struct Reflect<CCollisionEnter>
{
    static constexpr auto fields = std::make_tuple();
};

struct CCollisionExit
{
    static constexpr const char* name = "CollisionExit";

    Entity entity1;
    Entity entity2;

    CCollisionExit() = default;
    CCollisionExit(Entity a, Entity b) : entity1(a), entity2(b)
    {
    }
};

template <>
struct Reflect<CCollisionExit>
{
    static constexpr auto fields = std::make_tuple();
};

struct CCollisionStay
{
    static constexpr const char* name = "CollisionStay";

    Entity entity1;
    Entity entity2;

    CCollisionStay() = default;
    CCollisionStay(Entity a, Entity b) : entity1(a), entity2(b)
    {
    }
};

template <>
struct Reflect<CCollisionStay>
{
    static constexpr auto fields = std::make_tuple();
};
