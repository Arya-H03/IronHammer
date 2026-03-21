#pragma once
#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>

class Time
{
    friend class IronHammer;

private:
    inline static float m_deltaTime = 0;

public:
    static inline float DeltaTime() { return m_deltaTime; }

    inline static std::string GetLocalTimeStamp()
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::tm localTime = *std::localtime(&time);

        return std::format("[{:02}:{:02}:{:02}]", localTime.tm_hour, localTime.tm_min,
                           localTime.tm_sec);
    }

    inline static long long GetCurrentTimeMS()
    {
        std::chrono::time_point<std::chrono::steady_clock> timePoint =
            std::chrono::steady_clock::now();

        return std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch())
            .count();
    }
};
