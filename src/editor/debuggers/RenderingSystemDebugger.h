#pragma once

#include "editor/debuggers/IDebugTab.h"

class RenderingSystem;

class RenderingSystemDebugger : public IDebugTab
{
private:
    RenderingSystem* m_renderingSystemPtr;

public:
    RenderingSystemDebugger();

    void DrawTab(DebugTabContext& context) override;

    void RegisterRenderingSystem(RenderingSystem* renderingSystem);
    void UnRegisterRenderingSystem();
};
