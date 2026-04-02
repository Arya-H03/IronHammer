#pragma once

#include "ecs/World.hpp"
#include "editor/debuggers/IDebugTab.h"

class FlowFieldSystem;

class FlowFieldDebugger : public IDebugTab
{
private:
    std::vector<Entity> m_visualCells;

    bool m_toggleFlowField = true;

    FlowFieldSystem* m_flowFieldSystemPtr;

    void CreateVisualGrid(World* worldPtr);
    void ShowFlowField(World* worldPtr);
    void HideFlowField(World* worldPtr);

public:
    FlowFieldDebugger();

    void DrawTab(DebugTabContext& context) override;
    void ResetDebugger()override;

    void RegisterFlowField(FlowFieldSystem* flowFieldSystem, World* worldPtr);
    void UnRegisterFlowField();
};
