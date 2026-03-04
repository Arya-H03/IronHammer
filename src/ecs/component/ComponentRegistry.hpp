#pragma once
#include <algorithm>
#include <imgui.h>
#include <cstddef>
#include <cstring>
#include <cassert>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "core/utils/Debug.h"
#include "ecs/common/ECSCommon.h"
#include "nlohmann/json_fwd.hpp"

using DisplayComponentFn = void (*)(void*, bool*);
using MoveComponentFn = void (*)(void*, void*, size_t, size_t);
using SerializeComponentFn = void (*)(nlohmann::json&, void*);
using DeSerializeComponentFn = void* (*) (nlohmann::json&);
using EmplaceComponentFn = void (*)(void*, void*, size_t);
using DestroyComponentFn = void (*)(void*);

struct ComponentInfo
{
    ComponentID id;
    size_t size;
    const char* name;
    DisplayComponentFn DisplayComponent;
    MoveComponentFn MoveComponent;
    SerializeComponentFn SerializeComponent;
    DeSerializeComponentFn DeSerializeComponent;
    EmplaceComponentFn EmplaceComponent;
    DestroyComponentFn DestroyComponent;
};
struct PendingComponent
{
    const ComponentInfo* componentInfoPtr;
    void* componentDataPtr;
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
        newComponentInfo.DisplayComponent = [](void* ptr, bool* isDirty = nullptr)
        {
            ComponentType* component = reinterpret_cast<ComponentType*>(ptr);
            component->GuiInspectorDisplay(ptr, isDirty);
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
        newComponentInfo.SerializeComponent = [](nlohmann::json& json, void* ptr)
        {
            ComponentType* componentPtr = reinterpret_cast<ComponentType*>(ptr);
            json[ComponentType::name] = *componentPtr;
        };
        newComponentInfo.DeSerializeComponent = [](nlohmann::json& json) -> void*
        {
            if (!json.contains(ComponentType::name))
            {
                Log_Warning("Tried to deserialize json that doesn't contain required component");
                return nullptr;
            }

            ComponentType component {};
            from_json(json[ComponentType::name], component);

            // It will be funny to forget to delete this later
            return new ComponentType(std::move(component));
        };

        newComponentInfo.EmplaceComponent = [](void* arrayPtr, void* componentPtr, size_t index)
        {
            ComponentType* array = reinterpret_cast<ComponentType*>(arrayPtr);
            ComponentType* source = reinterpret_cast<ComponentType*>(componentPtr);

            new (&array[index]) ComponentType(std::move(*source));
        };

        newComponentInfo.DestroyComponent = [](void* componentPtr)
        {
            ComponentType* component = reinterpret_cast<ComponentType*>(componentPtr);
            component->~ComponentType();
            operator delete(component);
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

    static const ComponentInfo* GetComponentInfoPtrByName(const std::string name)
    {
        for (const auto& componentInfo : componentInfos)
        {
            if (componentInfo.name && name == componentInfo.name) return &componentInfo;
        }
        return nullptr;
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
