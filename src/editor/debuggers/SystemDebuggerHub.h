#pragma once

#include "editor/debuggers/CollisionDebugger.h"
#include "editor/debuggers/EditorDebugger.h"
#include "editor/debuggers/FlowFieldDebugger.h"
#include "editor/debuggers/IDebugTab.h"
#include "editor/debuggers/RenderingSystemDebugger.h"
#include "editor/debuggers/WorldDebugger.h"

#include <vector>

class SystemDebuggerHub
{
  private:
    CollisionDebugger m_collisionDebugger;
    WorldDebugger m_worldDebugger;
    RenderingSystemDebugger m_renderingSystemDebugger;
    EditorDebugger m_editorDebugger;
    FlowFieldDebugger m_flowFieldDebugger;

    std::vector<IDebugTab*> m_debugTabs;

    SystemDebuggerHub()
    {
        m_debugTabs.push_back(&m_worldDebugger);
        m_debugTabs.push_back(&m_renderingSystemDebugger);
        m_debugTabs.push_back(&m_editorDebugger);
        m_debugTabs.push_back(&m_collisionDebugger);
        m_debugTabs.push_back(&m_flowFieldDebugger);
    }

  public:
    SystemDebuggerHub(const SystemDebuggerHub&) = delete;
    SystemDebuggerHub& operator=(const SystemDebuggerHub&) = delete;
    SystemDebuggerHub(SystemDebuggerHub&&) noexcept = delete;
    SystemDebuggerHub& operator=(SystemDebuggerHub&&) noexcept = delete;

    static SystemDebuggerHub& Instance()
    {
        static SystemDebuggerHub instance;
        return instance;
    }

    const std::vector<IDebugTab*>& GetDebugTabs() const { return m_debugTabs; }

    CollisionDebugger& GetCollsionDebugger() { return m_collisionDebugger; }
    WorldDebugger& GetWorldDebugger() { return m_worldDebugger; }
    RenderingSystemDebugger& GetRenderignSystemDebugger() { return m_renderingSystemDebugger; }
    EditorDebugger& GetEditorDebugger() { return m_editorDebugger; }
    FlowFieldDebugger& GetFlowFieldDebugger() { return m_flowFieldDebugger; }
};
