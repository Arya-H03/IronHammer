#pragma once

#include "editor/entityInspector/componentGui/ComponentGui.h"
#include "game/GameComponents.hpp"

template <>
struct ComponentInspectorGui<CTower>
{
    static void Display(CTower& tower, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CTower>("Tower", &tower, RemoveComponentCallback, [&] { tower = CTower{}; }, isDirty);
    }
};

template <>
struct ComponentInspectorGui<CEnemy>
{
    static void Display(CEnemy& enemy, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CEnemy>("Enemy", &enemy, RemoveComponentCallback, [&] { enemy = CEnemy{}; }, isDirty);
    }
};

template <>
struct ComponentInspectorGui<CFlowFieldObstacle>
{
    static void Display(CFlowFieldObstacle& flowFieldObstacle, const std::function<void()>& RemoveComponentCallback,
                        bool* isDirty = nullptr)
    {
        ComponentHeader<CFlowFieldObstacle>(
            CFlowFieldObstacle::name, &flowFieldObstacle, RemoveComponentCallback, [&] { flowFieldObstacle = CFlowFieldObstacle{}; },
            isDirty);
    }
};

template <>
struct ComponentInspectorGui<CFlowFieldTarget>
{
    static void Display(CFlowFieldTarget& flowFieldTarget, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CFlowFieldTarget>(
            CFlowFieldTarget::name, &flowFieldTarget, RemoveComponentCallback, [&] { flowFieldTarget = CFlowFieldTarget{}; }, isDirty);
    }
};

template <>
struct ComponentInspectorGui<CFlowFieldAgent>
{
    static void Display(CFlowFieldAgent& flowFieldAgent, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CFlowFieldAgent>(
            CFlowFieldAgent::name, &flowFieldAgent, RemoveComponentCallback, [&] { flowFieldAgent = CFlowFieldAgent{}; }, isDirty);
    }
};
