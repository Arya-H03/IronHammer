#pragma once
#include <algorithm>
#include <imgui.h>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <string>
#include <typeinfo>
#include <vector>
#include "ecs/common/ECSCommon.h"

using DisplatComponentFn = void (*)(void*);
using MoveComponentFn = void (*)(void*, void*, size_t, size_t);

struct ComponentInfo
{
    ComponentID id;
    size_t size;
    const char* name;
    DisplatComponentFn DisplayComponent;
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

    template <typename TComponent>
    static void DrawDebugGUI(void* ptr)
    {
        TComponent& component = *reinterpret_cast<TComponent*>(ptr);
        component.GuiInspectorDisplay();
    }

    template <typename T>
    static ComponentID RegisterComponent()
    {
        static ComponentID id = []()
        {
            ComponentInfo newComponentInfo;

            newComponentInfo.id = MakeComponentID();
            assert(newComponentInfo.id < MaxComponents);
            newComponentInfo.name = typeid(T).name();
            newComponentInfo.size = sizeof(T);
            newComponentInfo.DisplayComponent = [](void* ptr)
                {
                    T* component = reinterpret_cast<T*>(ptr);
                    component->GuiInspectorDisplay(ptr);
                    //DrawDebugGUI<T>(ptr);
                };
            newComponentInfo.MoveComponent = [](void* src, void* dst, size_t srcIndex, size_t dstIndex)
            {
                T* srcArray = reinterpret_cast<T*>(src);
                T* dstArray = reinterpret_cast<T*>(dst);

                if constexpr (std::is_trivially_copyable_v<T>)
                {
                    // For trivial types, memcpy is safe and faster
                    std::memcpy(&dstArray[dstIndex], &srcArray[srcIndex], sizeof(T));
                }
                else
                {
                    // For non-trivial types, use move constructor
                    new (&dstArray[dstIndex]) T(std::move(srcArray[srcIndex]));
                    //srcArray[srcIndex].~T();
                }
            };

            if (componentInfos.size() <= newComponentInfo.id) componentInfos.resize(newComponentInfo.id + 1);
            componentInfos[newComponentInfo.id] = newComponentInfo;

            return newComponentInfo.id;
        }();

        return id;
    }

  public:

    template <typename T>
    static ComponentID GetComponentID()
    {
        static ComponentID id = RegisterComponent<T>();
        return id;
    }

    static const char* GetComponentNameById(ComponentID id) { return componentInfos[id].name; }

    template <typename TComponent>
    static const char* GetComponentNameByType(const TComponent& component)
    {
        ComponentID id = GetComponentID<TComponent>();
        return componentInfos[id].name;
    }
    template <typename TComponent>
    static const char* GetComponentNameByType()
    {
        ComponentID id = GetComponentID<TComponent>();
        return componentInfos[id].name;
    }

    template <typename TComponent>
    static std::string GetComponentDescription(const TComponent& component)
    {
        return component.GetDescription();
    }

    static const ComponentInfo& GetComponentInfoById(ComponentID id) { return componentInfos[id]; }
};
