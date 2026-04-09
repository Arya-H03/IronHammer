#include "MoldInstance.h"

#include "core/CoreComponents.hpp"
#include "ecs/World.hpp"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include "mold/MoldManager.h"

#include <cassert>
#include <vector>

MoldInstance::MoldInstance(Mold& sourceTemplate)
{
    m_name = sourceTemplate.entityName;
    DeserializeFrom(sourceTemplate.entityJson);
}

MoldInstance::~MoldInstance()
{
    Clear();
}

void MoldInstance::Clear()
{
    for (auto& [info, ptr] : m_components)
    {
        if (info && ptr) info->DestroyComponent(ptr);
    }

    m_components.clear();
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

        m_components.emplace_back(const_cast<ComponentInfo*>(componentInfo), componentPtr);
        m_presentComponents.insert(componentInfo->id);
    }
}

void MoldInstance::DrawInspector()
{
    for (auto& [info, ptr] : m_components)
    {
        if (info && ptr) info->DisplayComponent(ptr, [&]() { m_pendingRemovalcomponents.insert(info->id); }, &m_isDirty);
    }

    if (m_pendingRemovalcomponents.size() > 0)
    {
        std::erase_if(m_components, [&](const std::pair<const ComponentInfo*, void*>& pair)
                      { return m_pendingRemovalcomponents.contains(pair.first->id); });

        std::erase_if(m_presentComponents, [&](ComponentId id) { return m_pendingRemovalcomponents.contains(id); });
        m_pendingRemovalcomponents.clear();
    }

    if (m_pendingAdditionComponents.size() > 0)
    {
        for (const auto& [info, ptr] : m_pendingAdditionComponents)
        {
            m_components.emplace_back(info, ptr);
        }
        m_pendingAdditionComponents.clear();
    }
}

void MoldInstance::AddComponent(const ComponentInfo* info, void* ptr)
{
    if (m_presentComponents.contains(info->id)) return;
    m_pendingAdditionComponents.emplace_back(info, ptr);
    m_presentComponents.insert(info->id);
    m_isDirty = true;
}

void MoldInstance::Save(MoldManager& entityTemplateManager, World& world)
{
    if (!m_isDirty) return;

    Mold* entityTemplate = entityTemplateManager.GetMoldByName(m_name);
    assert(entityTemplate);

    for (const auto& entity : entityTemplate->derivedEntities)
    {
        for (auto& [info, ptr] : m_components)
        {
            void* liveEntityComponentPtr = world.TryGetComponent(entity, info->id);
            assert(liveEntityComponentPtr);
            void* entityTemplateComponentPtr = ptr;

            info->MoldComponent(liveEntityComponentPtr, entityTemplateComponentPtr);
        }
    }

    Json newEntityJson;
    for (auto& [info, ptr] : m_components)
    {
        if (info && ptr) info->SerializeComponent(newEntityJson, ptr);
    }

    entityTemplateManager.RemakeMoldWithJson(newEntityJson, m_name);
    m_isDirty = false;
}

void MoldInstance::Rename(MoldManager& entityTemplateManager, const std::string& newName, World& world)
{
    if (newName.empty() || newName == m_name) return;
    entityTemplateManager.ChangeMoldName(m_name, newName);
    m_name = newName;

    Mold* entityTemplate = entityTemplateManager.GetMoldByName(m_name);
    assert(entityTemplate);

    for (const auto& entity : entityTemplate->derivedEntities)
    {
        CMolded* moldDerived = world.TryGetComponent<CMolded>(entity);
        if (moldDerived)
        {
            moldDerived->moldName = newName;
        }
    }
}
