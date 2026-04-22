#pragma once
#include "ecs/World.h"
#include "editor/entityInspector/EntityInspector.h"
#include "engine/EngineMode.h"

struct DebugTabContext
{
    World* worldPtr;
    Inspector* entityInspectorPtr;
};
class IDebugTab
{
  protected:
    EngineMode m_displayMode = EngineMode::None;

  public:
    EngineMode GetDisplayMode() const { return m_displayMode; }
    virtual void DrawTab(DebugTabContext& context) = 0;
    virtual void ResetDebugger() {};
    virtual ~IDebugTab(){};
};
