#include "MoldInstance.h"

#include "core/CoreComponents.hpp"
#include "ecs/World.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "mold/MoldManager.h"

#include <cassert>
#include <vector>

MoldInstance::MoldInstance(Mold& sourceMold) : m_sourceMoldRef(sourceMold)
{
    DeserializeFrom(sourceMold.moldJson);
}

MoldInstance::~MoldInstance()
{
    Clear();
}

void MoldInstance::Clear()
{
    for (auto& [info, ptr] : m_moldInstanceComponents)
    {
        if (info && ptr) info->DestroyComponent(ptr);
    }

    m_moldInstanceComponents.clear();
    m_presentComponents.clear();
    m_isDirty = false;
}

void MoldInstance::DeserializeFrom(Json& entityJson)
{
    for (auto it = entityJson.begin(); it != entityJson.end(); ++it)
    {
        const std::string componentName = it.key();

        const ComponentInfo* componentInfo = ComponentRegistry::GetComponentInfoPtrByName(componentName);
        if (!componentInfo) continue;

        void* componentPtr = componentInfo->DeSerializeComponent(entityJson);
        if (!componentPtr) continue;

        m_moldInstanceComponents.emplace_back(const_cast<ComponentInfo*>(componentInfo), componentPtr);
        m_presentComponents.insert(componentInfo->id);
    }
}

void MoldInstance::DrawInspector()
{
    for (auto& [info, ptr] : m_moldInstanceComponents)
    {
        if (info && ptr) info->DisplayComponent(ptr, [&]() { RemoveComponentFromMoldInstance(info->id); }, &m_isDirty);
    }
}

void MoldInstance::AddComponentToMoldInstance(const ComponentInfo* info, void* ptr)
{
    if (m_presentComponents.contains(info->id)) return;

    m_presentComponents.insert(info->id);
    m_pendingAdditionComponents.emplace_back(info, ptr);
    m_moldInstanceComponents.emplace_back(info, ptr);

    m_isDirty = true;
}

void MoldInstance::RemoveComponentFromMoldInstance(ComponentId componentId)
{
    m_pendingRemovalcomponents.insert(componentId);
    std::erase_if(m_moldInstanceComponents,
                  [&](const std::pair<const ComponentInfo*, void*>& pair) { return m_pendingRemovalcomponents.contains(pair.first->id); });
    std::erase_if(m_presentComponents, [&](ComponentId id) { return m_pendingRemovalcomponents.contains(id); });

    m_isDirty = true;
}

void MoldInstance::SaveMoldsInstanceToMold(MoldManager& entityTemplateManager, World& world)
{
    if (!m_isDirty) return;

    for (const auto& entity : m_sourceMoldRef.derivedEntities)
    {
        // Remove Components from
        for (const auto compToRemoveId : m_pendingRemovalcomponents)
        {
            world.RemoveFromEntity(entity, compToRemoveId);
        }

        // Add Component to
        for (const auto& [compInfoPtr, compDataPtr] : m_pendingAdditionComponents)
        {
            void* newComponentPtr = compInfoPtr->DefaultConstructComponent();
            compInfoPtr->CopyComponent(newComponentPtr, compDataPtr);
            world.AddToEntity(entity, compInfoPtr->id, newComponentPtr);
        }

        // Apply component values
        for (auto& [compInfo, compPtr] : m_moldInstanceComponents)
        {
            void* liveEntityComponentPtr = world.TryGetComponent(entity, compInfo->id);
            if (liveEntityComponentPtr)
            {
                compInfo->MoldComponent(liveEntityComponentPtr, compPtr);
            }
        }
    }

    m_pendingRemovalcomponents.clear();
    m_pendingAdditionComponents.clear();

    // Rebuild Mold's Json object on the disk
    Json newEntityJson;
    for (auto& [info, ptr] : m_moldInstanceComponents)
    {
        if (info && ptr) info->SerializeComponent(newEntityJson, ptr);
    }

    entityTemplateManager.RemakeMoldWithJson(newEntityJson, m_sourceMoldRef.moldName);
    m_isDirty = false;
}

void MoldInstance::RenameMoldInstance(MoldManager& entityTemplateManager, const std::string& newName, World& world)
{
    if (newName.empty() || newName == m_sourceMoldRef.moldName) return;
    entityTemplateManager.ChangeMoldName(m_sourceMoldRef.moldName, newName);
    m_sourceMoldRef.moldName = newName;

    for (const auto& entity : m_sourceMoldRef.derivedEntities)
    {
        CMolded* moldDerived = world.TryGetComponent<CMolded>(entity);
        if (moldDerived)
        {
            moldDerived->moldName = newName;
        }
    }
}
