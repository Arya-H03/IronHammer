#pragma once

#include <vector>
#include <string>
#include <utility>
#include "core/saving/JsonUtility.h"

class EntityTemplate;
class ComponentInfo;
class EntityTemplateManager;

class EntityTemplateInstance
{
  private:

    std::vector<std::pair<ComponentInfo*, void*>> m_components;
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

    const std::string& GetName() const { return m_name; }
    bool IsDirty() const { return m_isDirty; }
};
