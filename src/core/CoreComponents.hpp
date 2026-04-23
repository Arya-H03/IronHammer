#pragma once
#include "core/reflection/ComponentReflection.h"
#include "core/utils/Vect2.hpp"

#include <string>
#include <tuple>

struct CTransform
{
    static constexpr const char* name = "Transform";

    Vect2f position = Vect2f{0, 0};
    Vect2f previousPosition = Vect2f{0, 0};
    Vect2f scale = Vect2f{1, 1};
    float rotation = 0.f;

    CTransform() = default;
    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot), previousPosition(pos)
    {
    }

    void OnAfterDeserialize()
    {
        previousPosition = position;
    }
};

template <>
struct Reflect<CTransform>
{
    static constexpr auto fields =
        std::make_tuple(Descriptor{"position", &CTransform::position, false}, Descriptor{"scale", &CTransform::scale, true},
                        Descriptor{"rotation", &CTransform::rotation, true});
};

struct CMovement
{
    static constexpr const char* name = "Movement";

    float speed = 0.f;

    CMovement() = default;
    CMovement(float spd) : speed(spd)
    {
    }
};

template <>
struct Reflect<CMovement>
{
    static constexpr auto fields = std::make_tuple(Descriptor{"speed", &CMovement::speed, true});
};

struct CMolded
{
    static constexpr const char* name = "Molded";

    std::string moldName = "";

    CMolded() = default;
    CMolded(const std::string& name) : moldName(name)
    {
    }
};

template <>
struct Reflect<CMolded>
{
    static constexpr auto fields = std::make_tuple(Descriptor{"moldName", &CMolded::moldName, true});
};
