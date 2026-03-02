#pragma once
#include <algorithm>
#include <imgui.h>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <vector>
#include "core/utils/Debug.h"
#include "ecs/common/ECSCommon.h"

using DisplayComponentFn = void (*)(void*);
using MoveComponentFn = void (*)(void*, void*, size_t, size_t);

struct ComponentInfo
{
    ComponentID id;
    size_t size;
    const char* name;
    DisplayComponentFn DisplayComponent;
    MoveComponentFn MoveComponent;
};

class ComponentRegistry
{
  private:

    inline static std::vector<ComponentInfo> componentInfos;

    static ComponentID MakeComponentID()
    {
        static ComponentID counter = 0;
        return counter++;
    }

    template <typename ComponentType>
    static ComponentID GetOrMakeComponentId()
    {
        static ComponentID id = MakeComponentID();
        return id;
    }

    template <typename ComponentType>
    static void DrawDebugGUI(void* ptr)
    {
        ComponentType& component = *reinterpret_cast<ComponentType*>(ptr);
        component.GuiInspectorDisplay();
    }

  public:

    template <typename ComponentType>
    static void RegisterComponent()
    {
        ComponentInfo newComponentInfo;

        newComponentInfo.id = GetOrMakeComponentId<ComponentType>();
        assert(newComponentInfo.id < MaxComponents);
        newComponentInfo.name = ComponentType::name;
        newComponentInfo.size = sizeof(ComponentType);
        newComponentInfo.DisplayComponent = [](void* ptr)
        {
            ComponentType* component = reinterpret_cast<ComponentType*>(ptr);
            component->GuiInspectorDisplay(ptr);
        };
        newComponentInfo.MoveComponent = [](void* src, void* dst, size_t srcIndex, size_t dstIndex)
        {
            ComponentType* srcArray = reinterpret_cast<ComponentType*>(src);
            ComponentType* dstArray = reinterpret_cast<ComponentType*>(dst);

            if constexpr (std::is_trivially_copyable_v<ComponentType>)
            {
                // For trivial types, memcpy is safe and faster
                std::memcpy(&dstArray[dstIndex], &srcArray[srcIndex], sizeof(ComponentType));
            }
            else
            {
                // For non-trivial types, use move constructor
                new (&dstArray[dstIndex]) ComponentType(std::move(srcArray[srcIndex]));
                // srcArray[srcIndex].~T();
            }
        };

        if (componentInfos.size() <= newComponentInfo.id) componentInfos.resize(newComponentInfo.id + 1);
        componentInfos[newComponentInfo.id] = newComponentInfo;
    }

    template <typename ComponentType>
    static ComponentID GetComponentID()
    {
        static ComponentID id = GetOrMakeComponentId<ComponentType>();
        return id;
    }

    static const char* GetComponentNameById(ComponentID id) { return componentInfos[id].name; }

    // Deprecated
    template <typename ComponentType>
    static const char* GetComponentNameByType(const ComponentType& component)
    {
        ComponentID id = GetComponentID<ComponentType>();
        return componentInfos[id].name;
    }
    // Deprecated
    template <typename ComponentType>
    static const char* GetComponentNameByType()
    {
        ComponentID id = GetComponentID<ComponentType>();
        return componentInfos[id].name;
    }

    static const ComponentInfo& GetComponentInfoById(ComponentID id) { return componentInfos[id]; }
};

template <typename ComponentType>
struct ComponentAutoRegister
{
    ComponentAutoRegister() { ComponentRegistry::RegisterComponent<ComponentType>(); }
};

#define REGISTER_COMPONENT(TYPE) inline static ComponentAutoRegister<TYPE> autoRegister_##TYPE;
