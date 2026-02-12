#pragma once
#include <imgui.h>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>
#include <typeinfo>
#include <vector>
#include "ECSCommon.h"

using DrawDebugGuiFn = void (*)(void*);

struct ComponentInfo
{
    ComponentID id;
    size_t size;
    const char* name;
    DrawDebugGuiFn drawGuiFn;
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

        ImGui::Text("%s: %s",
                    ComponentRegistry::GetComponentNameByType(component),
                    ComponentRegistry::GetComponentDescription(component).c_str());
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
            newComponentInfo.drawGuiFn = [](void* ptr) { DrawDebugGUI<T>(ptr); };

            if(componentInfos.size() <= newComponentInfo.id) componentInfos.resize(newComponentInfo.id + 1);
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

    static const ComponentInfo& GetComponentInfoById(ComponentID id)
    {
        return componentInfos[id];
    }

};
