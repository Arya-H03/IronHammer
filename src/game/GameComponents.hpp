#pragma once
#include "core/reflection/ComponentReflection.h"

struct CTower
{
    static constexpr const char* name = "Tower";

    CTower() = default;
};

template <>
struct Reflect<CTower>
{
    static constexpr auto fields = std::make_tuple();
};

struct CEnemy
{
    static constexpr const char* name = "Enemy";

    CEnemy() = default;
};

template <>
struct Reflect<CEnemy>
{
    static constexpr auto fields = std::make_tuple();
};
