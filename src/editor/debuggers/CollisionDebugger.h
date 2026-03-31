#pragma once
#include "ecs/World.hpp"
#include "editor/debuggers/IDebugTab.h"

#include <imgui.h>

class CollisionSystem;

class CollisionDebugger : public IDebugTab
{
private:
    CollisionSystem* m_collisionSystemPtr = nullptr;

    void BroadPhaseGui(World* worldPtr) const;
    void NarrowPhaseGui() const;

public:
    CollisionDebugger();

    void RegisterCollisionSystem(CollisionSystem* collisionSystem);
    void UnRegisterCollsionSystem();

    void DrawTab(DebugTabContext& context) override;
};
