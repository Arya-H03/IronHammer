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
