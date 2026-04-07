#pragma once
#include "core/CoreComponents.hpp"
#include "core/reflection/ComponentReflection.h"
#include "core/utils/Debug.h"
#include "ecs/common/ECSCommon.h"
#include "editor/entityInspector/componentGui/ComponentGui.h"
#include "editor/entityInspector/componentGui/CoreComponentsGui.h"
#include "editor/entityInspector/componentGui/GameComponentsGui.h"
#include "editor/entityInspector/componentGui/PhysicsComponentsGui.h"
#include "editor/entityInspector/componentGui/RenderingComponentsGui.h"
#include "game/GameComponents.hpp"
#include "nlohmann/json_fwd.hpp"
#include "physics/PhysicsComponents.hpp"
#include "rendering/RenderingComponents.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using Json = nlohmann::json;

using DisplayComponentFn          = void (*)(void*, const std::function<void()>&, bool*);
using MoveComponentFn             = void (*)(void*, void*, size_t, size_t);
using SerializeComponentFn        = void (*)(nlohmann::json&, void*);
using DeSerializeComponentFn      = void* (*)(nlohmann::json&);
using EmplaceComponentFn          = void (*)(void*, void*, size_t);
using DefaultConstructComponentFn = void* (*)();
using DestroyComponentFn          = void (*)(void*);

struct ComponentInfo
{
    ComponentId                 id;
    size_t                      size;
    const char*                 name;
    DisplayComponentFn          DisplayComponent;
    MoveComponentFn             MoveComponent;
    SerializeComponentFn        SerializeComponent;
    DeSerializeComponentFn      DeSerializeComponent;
    EmplaceComponentFn          EmplaceComponent;
    DefaultConstructComponentFn DefaultConstructComponent;
    DestroyComponentFn          DestroyComponent;
};
struct PendingComponent
{
    const ComponentInfo* componentInfoPtr = nullptr;

    void* componentDataPtr = nullptr;

    PendingComponent() = default;
    PendingComponent(const ComponentInfo* info, void* data) : componentInfoPtr(info), componentDataPtr(data) {}

    PendingComponent(PendingComponent&& other) noexcept
        : componentInfoPtr(other.componentInfoPtr), componentDataPtr(other.componentDataPtr)
    {
        other.componentDataPtr = nullptr;
        other.componentInfoPtr = nullptr;
    }

    PendingComponent& operator=(PendingComponent&& other) noexcept
    {
        if (this != &other) {
            if (componentInfoPtr) componentInfoPtr->DestroyComponent(componentDataPtr);

            componentInfoPtr = other.componentInfoPtr;
            componentDataPtr = other.componentDataPtr;

            other.componentInfoPtr = nullptr;
            other.componentDataPtr = nullptr;
        }
        return *this;
    }

    PendingComponent(const PendingComponent&)            = delete;
    PendingComponent& operator=(const PendingComponent&) = delete;

    ~PendingComponent()
    {
        if (componentInfoPtr) const_cast<ComponentInfo*>(componentInfoPtr)->DestroyComponent(componentDataPtr);
    }
};

class ComponentRegistry
{
private:
    inline static std::vector<ComponentInfo> componentInfos;

    ComponentRegistry() = delete;

    static ComponentId MakeComponentID()
    {
        static ComponentId counter = 0;
        return counter++;
    }

    template <typename ComponentType>
    static ComponentId GetOrMakeComponentId()
    {
        static ComponentId id = MakeComponentID();
        return id;
    }

    template <typename Component>
    static void RegisterComponent()
    {
        ComponentInfo newComponentInfo;

        newComponentInfo.id = GetOrMakeComponentId<Component>();
        assert(newComponentInfo.id < MaxComponents);

        newComponentInfo.name = Component::name;
        newComponentInfo.size = sizeof(Component);

        newComponentInfo.DisplayComponent = [](void* ptr, const std::function<void()>& RemoveComponentCallback,
                                               bool* isDirty = nullptr) {
            Component* component = reinterpret_cast<Component*>(ptr);
            ComponentInspectorGui<Component>::Display(*component, RemoveComponentCallback, isDirty);
        };

        newComponentInfo.MoveComponent = [](void* src, void* dst, size_t srcIndex, size_t dstIndex) {
            Component* srcArray = reinterpret_cast<Component*>(src);
            Component* dstArray = reinterpret_cast<Component*>(dst);

            if constexpr (std::is_trivially_copyable_v<Component>) {
                // For trivial types, memcpy is safe and faster
                std::memcpy(&dstArray[dstIndex], &srcArray[srcIndex], sizeof(Component));
            }
            else {
                // For non trivial types, use move constructor
                new (&dstArray[dstIndex]) Component(std::move(srcArray[srcIndex]));
                // srcArray[srcIndex].~T();
            }
        };
        newComponentInfo.SerializeComponent = [](Json& entityJson, void* ptr) {
            Component*       componentPtr  = reinterpret_cast<Component*>(ptr);
            Json&            componentJson = entityJson[Component::name];
            SerializeVisitor visitor{componentJson};
            ReflectVisit(*componentPtr, visitor);
        };
        newComponentInfo.DeSerializeComponent = [](Json& entityJson) -> void* {
            if (!entityJson.contains(Component::name)) {
                LOG_WARNING("Tried to deserialize json that doesn't contain required component");
                return nullptr;
            }

            Component          component{};
            Json&              componentJson = entityJson[Component::name];
            DeSerializeVisitor visitor{componentJson};
            ReflectVisit(component, visitor);

            if constexpr (requires { component.OnAfterDeserialize(); }) { component.OnAfterDeserialize(); }

            // It will be funny to forget to delete this later
            return new Component(std::move(component));
        };

        newComponentInfo.EmplaceComponent = [](void* arrayPtr, void* componentPtr, size_t index) {
            Component* array  = reinterpret_cast<Component*>(arrayPtr);
            Component* source = reinterpret_cast<Component*>(componentPtr);

            // It will be moved into allocator, no need to delete manually.
            new (&array[index]) Component(std::move(*source));
        };

        newComponentInfo.DestroyComponent = [](void* componentPtr) {
            Component* component = reinterpret_cast<Component*>(componentPtr);
            component->~Component();
            operator delete(component);
        };

        newComponentInfo.DefaultConstructComponent = []() -> void* {
            // It will be funny to forget to delete this later
            return new Component();
        };

        if (componentInfos.size() <= newComponentInfo.id) componentInfos.resize(newComponentInfo.id + 1);
        componentInfos[newComponentInfo.id] = newComponentInfo;
    }

public:
    inline static const std::vector<ComponentInfo>& GetComponentInfos() { return componentInfos; }

    static const char* GetComponentNameById(ComponentId id) { return componentInfos[id].name; }

    static void RegisterAllComponents()
    {
        RegisterComponent<CTransform>();
        RegisterComponent<CMovement>();
        RegisterComponent<CCollider>();
        RegisterComponent<CRigidBody>();
        RegisterComponent<CCollisionEnter>();
        RegisterComponent<CCollisionExit>();
        RegisterComponent<CCollisionStay>();
        RegisterComponent<CSprite>();
        RegisterComponent<CShape>();
        RegisterComponent<CNotDrawable>();
        RegisterComponent<CText>();
        RegisterComponent<CTower>();
        RegisterComponent<CEnemy>();
        RegisterComponent<CFlowFieldObstacle>();
        RegisterComponent<CFlowFieldTarget>();
        RegisterComponent<CFlowFieldAgent>();
    }

    template <typename ComponentType>
    static ComponentId GetComponentID()
    {
        static ComponentId id = GetOrMakeComponentId<ComponentType>();
        return id;
    }

    static const ComponentInfo* GetComponentInfoPtrByName(const std::string name)
    {
        for (const auto& componentInfo : componentInfos) {
            if (componentInfo.name && name == componentInfo.name) return &componentInfo;
        }
        return nullptr;
    }

    template <typename... Components>
    static ComponentSignatureMask MakeSignatureMask()
    {
        ComponentSignatureMask signature;
        (signature.set(ComponentRegistry::GetComponentID<Components>()), ...);
        return signature;
    }

    static ComponentSignatureMask MakeSignatureMask(std::vector<PendingComponent>& pendingComponents)
    {
        ComponentSignatureMask signature;
        for (const auto& component : pendingComponents) { signature.set(component.componentInfoPtr->id); }
        return signature;
    }

    inline static std::vector<PendingComponent> GetAllPendingComponentsFromEntityJson(Json& entityJson)
    {
        std::vector<PendingComponent> pendingComponents;

        for (auto it = entityJson.begin(); it != entityJson.end(); ++it) {
            const std::string    componentName = it.key();
            const ComponentInfo* componentInfo = ComponentRegistry::GetComponentInfoPtrByName(componentName);
            if (!componentInfo) {
                LOG_WARNING("Unknown component during deserialization: " + componentName);
                continue;
            }

            void* componentPtr = componentInfo->DeSerializeComponent(entityJson);
            if (!componentPtr) {
                LOG_WARNING("Failed to deserialize component: " + componentName);
                continue;
            }
            pendingComponents.emplace_back(componentInfo, componentPtr);
        }
        return pendingComponents;
    }

    template <typename Component>
    static PendingComponent GetPendingComponentFromEntityJson(Json& entityJson)
    {
        std::string name = GetComponentNameByType<Component>();

        for (auto it = entityJson.begin(); it != entityJson.end(); ++it) {
            if (it.key() != name) continue;

            const ComponentInfo* info = ComponentRegistry::GetComponentInfoPtrByName(name);
            if (!info) return PendingComponent();

            void* data = info->DeSerializeComponent(entityJson);
            if (!data) return PendingComponent();

            return PendingComponent(info, data);
        }
        return PendingComponent();
    }

    template <typename T>
    static T* GetComponentFromPendings(const std::vector<PendingComponent>& pendingComponent)
    {
        for (auto& component : pendingComponent) {
            if (component.componentInfoPtr->id == ComponentRegistry::GetComponentID<T>()) {
                T* componentPtr = reinterpret_cast<T*>(component.componentDataPtr);
                return componentPtr;
            }
        }
        return nullptr;
    }

    template <typename T>
    static T* GetComponentFromPending(const PendingComponent& pendingComponent)
    {
        if (!pendingComponent.componentDataPtr || !pendingComponent.componentInfoPtr) return nullptr;
        if (pendingComponent.componentInfoPtr->id == ComponentRegistry::GetComponentID<T>()) {
            T* componentPtr = reinterpret_cast<T*>(pendingComponent.componentDataPtr);
            return componentPtr;
        }

        return nullptr;
    }

    static const ComponentInfo& GetComponentInfoById(ComponentId id) { return componentInfos[id]; }

    // Deprecated
    template <typename ComponentType>
    static const char* GetComponentNameByType(const ComponentType& component)
    {
        ComponentId id = GetComponentID<ComponentType>();
        return componentInfos[id].name;
    }
    // Deprecated
    template <typename ComponentType>
    static const char* GetComponentNameByType()
    {
        ComponentId id = GetComponentID<ComponentType>();
        return componentInfos[id].name;
    }
};
