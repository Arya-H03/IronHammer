#pragma once

#include "core/saving/JsonUtility.h"
#include "ecs/common/ECSCommon.h"

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

class EntityTemplate;
class ComponentInfo;
class EntityTemplateManager;

class EntityTemplateInstance
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
    explicit EntityTemplateInstance(EntityTemplate& sourceTemplate);
    ~EntityTemplateInstance();

    EntityTemplateInstance(const EntityTemplateInstance&) = delete;
    EntityTemplateInstance& operator=(const EntityTemplateInstance&) = delete;

    void DrawInspector();
    void Save(EntityTemplateManager& entityTemplateManager);
    void Rename(EntityTemplateManager& entityTemplateManager, const std::string& newName);
    void AddComponent(const ComponentInfo* info, void* ptr);

    const std::string& GetName() const { return m_name; }
    bool IsDirty() const { return m_isDirty; }
};
