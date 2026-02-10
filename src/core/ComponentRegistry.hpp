#pragma once
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>
#include <unordered_map>
#include <typeinfo>

static constexpr size_t MaxComponents = 32;
using ComponentID = uint32_t;

class ComponentRegistry
{
  private:
    static ComponentID MakeComponentID()
    {
        static ComponentID counter = 0;
        return counter++;
    }

    inline static std::unordered_map<ComponentID, std::string> componentIdToStringMap;

  public:
    template <typename T>
    static ComponentID GetComponentID()
    {
        static ComponentID id = []()
        {
            ComponentID newId = MakeComponentID();
            assert(newId < MaxComponents);
            componentIdToStringMap[newId] = GetComponentNameByType<T>();
            return newId;
        }();

        return id;
    }

    static std::string& GetComponentNameById(ComponentID id)
    {
        return componentIdToStringMap[id];
    }

    template <typename TComponent>
    static std::string GetComponentNameByType(const TComponent& component)
    {
        const char* name = typeid(TComponent).name();
        return name + 1;
    }
    template <typename TComponent>
    static std::string GetComponentNameByType()
    {
        const char* name = typeid(TComponent).name();
        return name + 1;
    }

    template <typename TComponent>
    static std::string GetComponentInfo(const TComponent& component)
    {
        return component.GetInfo();
    }
};
