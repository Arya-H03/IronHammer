#pragma once

#include "core/saving/JsonUtility.h"
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

class Mold;
class ComponentInfo;
class MoldManager;

class MoldInstance
{
  private:
    std::vector<std::pair<const ComponentInfo*, void*>> m_components;
    std::unordered_set<ComponentId> m_presentComponents;

    std::unordered_set<ComponentId> m_pendingRemovalcomponents;
    std::vector<std::pair<const ComponentInfo*, void*>> m_pendingAdditionComponents;

    std::string m_name;
    bool m_isDirty = false;

    void DeserializeFrom(Json& j);
    void Clear();

  public:
    explicit MoldInstance(Mold& sourceTemplate);
    ~MoldInstance();

    MoldInstance(const MoldInstance&) = delete;
    MoldInstance& operator=(const MoldInstance&) = delete;

    void DrawInspector();
    void Save(MoldManager& entityTemplateManager, World& world);
    void Rename(MoldManager& entityTemplateManager, const std::string& newName, World& world);
    void AddComponent(const ComponentInfo* info, void* ptr);

    const std::string& GetName() const
    {
        return m_name;
    }
    bool IsDirty() const
    {
        return m_isDirty;
    }
};
