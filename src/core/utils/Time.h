#pragma once
#include <ctime>
#include <string>
#include <chrono>

class Time
{
  private:
  public:

    inline static std::string GetLocalTimeStamp()
    {
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm localTime = *std::localtime(&time);

        return std::format("[{:02}:{:02}:{:02}]", localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
    }
};
