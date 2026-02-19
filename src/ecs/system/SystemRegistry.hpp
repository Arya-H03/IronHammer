#pragma once
#include <algorithm>
#include <concepts>
#include <memory>
#include <utility>
#include <vector>
#include "ecs/system/BaseSystem.h"

// Deprecated
class SystemRegistry
{
  private:

    std::vector<std::unique_ptr<BaseSystem>> m_systems;

  public:

    const std::vector<std::unique_ptr<BaseSystem>>& GetSystems() const { return m_systems; }

    template <std::derived_from<BaseSystem> System, typename... Args>
    System& RegisterSystem(Args&&... args)
    {
        std::unique_ptr<System> newSystem = std::make_unique<System>(std::forward<Args>(args)...);
        System& ref = *newSystem;
        m_systems.push_back(std::move(newSystem));
        return ref;
    }
};
