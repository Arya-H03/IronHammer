#pragma once

#include "core/utils/Time.h"
#include "editor/Editor.h"
#include "engine/Engine.h"

#include <algorithm>
#include <chrono>
#include <thread>

class IronHammer
{
private:
    Engine m_engine;
    Editor m_editor;

public:
    IronHammer() : m_editor(m_engine) {}

    void Run()
    {

        long long startTime = Time::GetCurrentTimeMS();
        while (m_engine.GetRenderWindow().isOpen())
        {
            m_engine.BeginFrame();
            m_engine.UpdateRuntime();
            m_editor.Update();
            m_engine.RenderFrame();
            m_engine.EndFrame();
            Debug::FlushLogs();

            long long elapsedTime     = Time::GetCurrentTimeMS() - startTime;
            long long targetFrameTime = m_engine.GetTargetFrameTime();

            if (elapsedTime < targetFrameTime)
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(std::max(0LL, targetFrameTime - elapsedTime)));
            }

            long long now            = Time::GetCurrentTimeMS();
            long long totalFrameTime = now - startTime;
            startTime                = now;

            float sec         = totalFrameTime / 1000.f;
            sec               = std::min(sec, 0.05f);
            Time::m_deltaTime = Time::DeltaTime() * 0.1f + sec * 0.9f;
        }

        Debug::ShutdownLoggerThread();
    }
};
