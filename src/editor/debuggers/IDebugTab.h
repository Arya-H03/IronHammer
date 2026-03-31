#pragma once
#include "ecs/entity/EntityInspector.h"
#include "ecs/World.hpp"
#include "engine/Engine.h"

struct DebugTabContext
{
    World*           worldPtr;
    EntityInspector* entityInspectorPtr;
};
class IDebugTab
{
protected:
    EngineMode m_displayMode = EngineMode::None;

public:
    EngineMode   GetDisplayMode() const { return m_displayMode; }
    virtual void DrawTab(DebugTabContext& context) = 0;
    virtual ~IDebugTab(){};
};
