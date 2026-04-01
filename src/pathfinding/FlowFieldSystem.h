#pragma once

#include "core/utils/Vect2.hpp"
#include "ecs/system/ISystem.h"
#include "editor/debuggers/FlowFieldDebugger.h"
#include "editor/debuggers/SystemDebuggerHub.h"
#include "pathfinding/FlowField.h"
#include "pathfinding/FlowFieldGenerator.h"

class FlowFieldSystem : ISetupSystem
{
    friend class FlowFieldDebugger;

private:
    FlowFieldGenerator m_flowFieldGenerator;
    FlowField          m_flowField;

public:
    FlowFieldSystem() { m_flowField = m_flowFieldGenerator.GenerateFlowField(Vect2f{0, 0}, 1250, 1250, 25); }
    ~FlowFieldSystem() { SystemDebuggerHub::Instance().GetFlowFieldDebugger().UnRegisterFlowField(); }

    void SetupSystem(World* worldPtr) override
    {
        SystemDebuggerHub::Instance().GetFlowFieldDebugger().RegisterFlowField(this, worldPtr);
    }
};
