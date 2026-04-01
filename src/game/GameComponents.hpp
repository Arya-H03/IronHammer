#pragma once
#include "ecs/component/ComponentRegistry.hpp"
#include "ecs/entity/EntityInspectorHelper.h"

#include <nlohmann/json.hpp>

using namespace EntityInspectorHelpers;
using Json = nlohmann::json;

struct CTower
{
    static constexpr const char* name = "Tower";

    CTower() = default;
    REGISTER_COMPONENT(CTower);

    void Reset() {};

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CTower>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CTower&) { j = Json::object(); }
inline void from_json(const Json&, CTower&) {}

struct CEnemy
{
    static constexpr const char* name = "Enemy";

    CEnemy() = default;
    REGISTER_COMPONENT(CEnemy);

    void Reset() {};

    void GuiInspectorDisplay(void* ptr, const std::function<void()>& RemoveComponentCallback, bool* isDirty = nullptr)
    {
        ComponentHeader<CEnemy>(name, ptr, RemoveComponentCallback, [this] { Reset(); }, isDirty);
    }
};

inline void to_json(Json& j, const CEnemy&) { j = Json::object(); }
inline void from_json(const Json&, CEnemy&) {}
