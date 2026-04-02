#pragma once
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"
#include "core/reflection/ComponentReflection.h"


using namespace EntityInspectorHelpers;

struct CTower
{
    static constexpr const char* name = "Tower";

    CTower() = default;
    REGISTER_COMPONENT(CTower);

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CTower>(name, ptr, RemoveComponentCallback, [this] { *this = CTower{}; }, isDirty);
    }
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
    REGISTER_COMPONENT(CEnemy);

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CEnemy>(name, ptr, RemoveComponentCallback, [this] { *this = CEnemy{}; }, isDirty);
    }
};

template <>
struct Reflect<CEnemy>
{
    static constexpr auto fields = std::make_tuple();
};
