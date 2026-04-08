#pragma once
#include "core/reflection/ComponentReflection.h"
#include "core/utils/Vect2.hpp"

struct CTransform
{
    static constexpr const char* name = "Transform";

    Vect2f position = Vect2f(0, 0);
    Vect2f scale = Vect2f(1, 1);
    float rotation = 0.f;

    CTransform() = default;
    CTransform(const Vect2f& pos, const Vect2f& scl, float rot) : position(pos), scale(scl), rotation(rot) {}
};

template <>
struct Reflect<CTransform>
{
    static constexpr auto fields = std::make_tuple(std::pair{"position", &CTransform::position}, std::pair{"scale", &CTransform::scale},
                                                   std::pair{"rotation", &CTransform::rotation});
};

struct CMovement
{
    static constexpr const char* name = "Movement";

    float speed = 0.f;

    CMovement() = default;
    CMovement(float spd) : speed(spd) {}
};

template <>
struct Reflect<CMovement>
{
    static constexpr auto fields = std::make_tuple(std::pair{"speed", &CMovement::speed});
};
