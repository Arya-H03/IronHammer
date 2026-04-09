#pragma once

#include "ecs/World.h"
#include "editor/debuggers/IDebugTab.h"

class FlowFieldSystem;

class FlowFieldDebugger : public IDebugTab
{
  private:
    std::vector<Entity> m_flowFieldCellVisuals;
    std::vector<Entity> m_flowFieldArrowVisuals;

    bool m_toggleFlowField = true;
    bool m_toggleFlowFieldCells = true;
    bool m_toggleFlowFieldArrows = true;

    FlowFieldSystem* m_flowFieldSystemPtr;

    void CreateVisualGrid(World* worldPtr);
    void ShowFlowField(World* worldPtr);
    void HideFlowField(World* worldPtr);
    void ShowFlowFieldCells(World* worldPtr);
    void HideFlowFieldCells(World* worldPtr);
    void ShowFlowFieldArrows(World* worldPtr);
    void HideFlowFieldArrows(World* worldPtr);

  public:
    FlowFieldDebugger();

    void DrawTab(DebugTabContext& context) override;
    void ResetDebugger() override;

    void RegisterFlowField(FlowFieldSystem* flowFieldSystem, World* worldPtr);
    void UpdateFlowFieldVisualCells(World* worldPtr);
    void UnRegisterFlowField();
};
