#pragma once

#include "core/utils/Time.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <thread>

class FrameRateHandler
{
  private:
    static inline size_t m_targetFrameRate = 60; // 0 for unlimited
    static inline size_t m_currentFrameRate = 0;
    static inline size_t m_currentFrame = 0;
    static inline float m_accumulator = 0;

    static inline size_t m_fixedFrameRate = 50;

    long long m_currentFrameStartTime = 0; // in microsecond

    void Wait()
    {
        if (m_targetFrameRate == 0) return;

        long long targetFrameTime = 1000000 / m_targetFrameRate;
        long long targetTime = m_currentFrameStartTime + targetFrameTime;

        long long now = Time::GetCurrentTimeInMicrosecond();
        long long remaining = targetTime - now;

        if (remaining > 2000)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(remaining - 1000));
        }

        while (Time::GetCurrentTimeInMicrosecond() < targetTime)
        {
        }
    }

  public:
    FrameRateHandler()
    {
        Time::m_fixedDeltaTime = 1.f / m_fixedFrameRate;
    }

    static size_t GetTargetFrameRate()
    {
        return m_targetFrameRate;
    }
    static void SetTargetFrameRate(size_t targetFrameRate)
    {
        m_targetFrameRate = targetFrameRate;
    }
    static size_t GetCurrentFrameRate()
    {
        return m_currentFrameRate;
    }
    static size_t GetCurrentFrame()
    {
        return m_currentFrame;
    }
    static float GetRenderAlpha()
    {
        return m_accumulator / Time::m_fixedDeltaTime;
    }

    void OnFrameBegin()
    {
        m_currentFrameStartTime = Time::GetCurrentTimeInMicrosecond();

        m_accumulator += Time::m_rawDeltaTime;
        const float maxAccumulation = 0.25f;
        m_accumulator = std::min(m_accumulator, maxAccumulation);
    }

    void OnFrameEnd()
    {
        Wait();

        long long now = Time::GetCurrentTimeInMicrosecond();
        long long totalFrameTime = now - m_currentFrameStartTime;

        if (totalFrameTime > 0) m_currentFrameRate = 1000000 / totalFrameTime;

        m_currentFrameStartTime = now;

        float sec = totalFrameTime / 1000000.f;
        sec = std::min(sec, 0.05f);
        Time::m_rawDeltaTime = sec;
        Time::m_deltaTime = Time::DeltaTime() * 0.1f + sec * 0.9f;

        ++m_currentFrame;
    }

    bool CanUpdatePhysics()
    {
        return m_accumulator >= Time::FixedDeltaTime();
    }

    void OnAfterPhysicsUpdate()
    {
        m_accumulator -= Time::FixedDeltaTime();
    }
};
