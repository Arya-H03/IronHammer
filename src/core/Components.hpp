#include <cstdint>
#include <cassert>

using ComponentID = uint32_t;
static constexpr size_t MaxComponents = 32;

ComponentID MakeComponentID() {
    static ComponentID counter = 0;
    return counter++;
}

template<typename T>
ComponentID GetComponentID() {
    static ComponentID id = []() {
        ComponentID newId = MakeComponentID();
        assert(newId < MaxComponents);
        return newId;
    }();

    return id;
}


struct CPosition {
    float x, y, z;
};

struct CVelocity {
    float x, y, z;
};

struct CRotation {
    float x, y, z;
};
