#pragma once

#include "editor/Editor.h"
#include "engine/Engine.h"


class IronHammer
{
private:
    Engine m_engine;
    Editor m_editor;

public:
    IronHammer() : m_editor(m_engine) {}

    void Run()
    {

        while (m_engine.GetRenderWindow().isOpen())
        {
            m_engine.BeginFrame();
            m_engine.UpdateRuntime();
            m_editor.Update();
            m_engine.RenderFrame();
            Debug::FlushLogs();
            m_engine.EndFrame();
        }

        Debug::ShutdownLoggerThread();
    }
};
