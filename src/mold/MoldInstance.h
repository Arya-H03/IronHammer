#pragma once

#include "core/saving/JsonUtility.h"
#include "ecs/World.h"
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
    std::vector<std::pair<const ComponentInfo*, void*>> m_moldInstanceComponents;
    std::unordered_set<ComponentId> m_presentComponents;

    std::unordered_set<ComponentId> m_pendingRemovalcomponents;
    std::vector<std::pair<const ComponentInfo*, void*>> m_pendingAdditionComponents;

    Mold& m_sourceMoldRef;
    bool m_isDirty = false;

    void DeserializeFrom(Json& j);
    void Clear();

    void RemoveComponentFromMoldInstance(ComponentId componentId);

  public:
    explicit MoldInstance(Mold& sourceMold);
    ~MoldInstance();

    MoldInstance(const MoldInstance&) = delete;
    MoldInstance& operator=(const MoldInstance&) = delete;

    void DrawInspector();

    void SaveMoldsInstanceToMold(MoldManager& entityTemplateManager, World& world);
    void RenameMoldInstance(MoldManager& entityTemplateManager, const std::string& newName, World& world);
    void AddComponentToMoldInstance(const ComponentInfo* info, void* ptr);

    const std::string& GetName() const
    {
        return m_sourceMoldRef.moldName;
    }
    bool IsDirty() const
    {
        return m_isDirty;
    }
};
