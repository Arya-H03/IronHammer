#include "EntityTemplateInstance.h"
#include "EntityTemplateManager.h"
#include "ecs/common/ECSCommon.h"
#include "ecs/component/ComponentRegistry.hpp"
#include <vector>

EntityTemplateInstance::EntityTemplateInstance(EntityTemplate& sourceTemplate)
{
    m_name = sourceTemplate.entityName;
    DeserializeFrom(sourceTemplate.entityJson);
}

EntityTemplateInstance::~EntityTemplateInstance() { Clear(); }

void EntityTemplateInstance::Clear()
{
    for (auto& [info, ptr] : m_components)
    {
        if (info && ptr) info->DestroyComponent(ptr);
    }

    m_components.clear();
    m_presentComponents.clear();
    m_isDirty = false;
}

void EntityTemplateInstance::DeserializeFrom(Json& entityJson)
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

void EntityTemplateInstance::DrawInspector()
{
    for (auto& [info, ptr] : m_components)
    {
        if (info && ptr) info->DisplayComponent(ptr, [&]() { m_pendingRemovalcomponents.insert(info->id); }, &m_isDirty);
    }

    if (m_pendingRemovalcomponents.size() > 0)
    {
        std::erase_if(
            m_components, [&](const std::pair<const ComponentInfo*, void*>& pair) { return m_pendingRemovalcomponents.contains(pair.first->id); });

        std::erase_if(
            m_presentComponents, [&](ComponentId id) { return m_pendingRemovalcomponents.contains(id); });
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

void EntityTemplateInstance::AddComponent(const ComponentInfo* info, void* ptr)
{
    if (m_presentComponents.contains(info->id)) return;
    m_pendingAdditionComponents.emplace_back(info, ptr);
    m_presentComponents.insert(info->id);
    m_isDirty = true;
}

void EntityTemplateInstance::Save(EntityTemplateManager& entityTemplateManager)
{
    if (!m_isDirty) return;

    Json newEntityJson;
    for (auto& [info, ptr] : m_components)
    {
        if (info && ptr) info->SerializeComponent(newEntityJson, ptr);
    }
    entityTemplateManager.UpdateEntityTemplate(newEntityJson, m_name);
    m_isDirty = false;
}

void EntityTemplateInstance::Rename(EntityTemplateManager& entityTemplateManager, const std::string& newName)
{
    if (newName.empty() || newName == m_name) return;
    entityTemplateManager.ChangeEntityTemplateName(m_name, newName);
    m_name = newName;
}
