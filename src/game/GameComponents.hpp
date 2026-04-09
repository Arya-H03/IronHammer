#pragma once
#include "core/reflection/ComponentReflection.h"

#include <utility>

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

struct CFlowFieldObstacle
{
    static constexpr const char* name = "FlowFieldObstacle";

    CFlowFieldObstacle() = default;
};

template <>
struct Reflect<CFlowFieldObstacle>
{
    static constexpr auto fields = std::make_tuple();
};

struct CFlowFieldTarget
{
    static constexpr const char* name = "FlowFieldTarget";

    CFlowFieldTarget() = default;
};

template <>
struct Reflect<CFlowFieldTarget>
{
    static constexpr auto fields = std::make_tuple();
};

struct CFlowFieldAgent
{
    static constexpr const char* name = "FlowFieldAgent";

    float speed = 0.f;

    CFlowFieldAgent() = default;
    CFlowFieldAgent(float spd) : speed(spd)
    {
    }

    void Reset()
    {
        speed = 0;
    }
};

template <>
struct Reflect<CFlowFieldAgent>
{
    static constexpr auto fields = std::make_tuple(Descriptor{"speed", &CFlowFieldAgent::speed, true});
};
