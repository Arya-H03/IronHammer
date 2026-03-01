#pragma once

#include "engine/Engine.h"
#include "editor/Editor.h"

class IronHammer
{
  private:

    Engine m_engine;
    Editor m_editor;

  public:

    IronHammer() : m_editor(m_engine) { }

    void Run()
    {
        while (m_engine.GetRenderWindow().isOpen())
        {
            float dt = m_engine.BeginFrame();
            m_engine.UpdateRuntime();

            EditorFrameContext editorFrameContext(m_engine.GetRenderSystem(), m_engine.GetInputSystem(), m_engine.GetCurrentWorld());
            m_editor.Update(editorFrameContext);

            m_engine.RenderFrame();
            m_engine.EndFrame();
        }
    }
};
