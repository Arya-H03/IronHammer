#pragma once
#include <cstddef>
#include <deque>
#include <string>
#include <vector>
#include <imgui.h>
#include "core/utils/Colors.h"
#include "core/utils/Time.h"

enum class LogType
{
    Info,
    Warning,
    Error
};

struct LogMessage
{
    std::string message;
    std::string time;
    ImVec4 color;
    LogType logType;
};

class Debug
{
  private:

    inline static std::deque<LogMessage> m_logMessages;
    inline static constexpr size_t m_logLimit = 1000;

  public:

    inline static const std::deque<LogMessage>& GetLogMessages() { return m_logMessages; }
    inline static void Clear() { m_logMessages.clear(); }

    static void Log(const std::string& message)
    {
        if (m_logMessages.size() >= m_logLimit) m_logMessages.pop_front();
        m_logMessages.push_back({ message, Time::GetLocalTimeStamp(), Colors::ConcreteGrey_ImGui, LogType::Info });
    }
    static void Warning(const std::string& message)
    {
        if (m_logMessages.size() >= m_logLimit) m_logMessages.pop_front();
        m_logMessages.push_back({ message, Time::GetLocalTimeStamp(), Colors::HazardYellow_ImGui, LogType::Warning });
    }
    static void Error(const std::string& message)
    {
        if (m_logMessages.size() >= m_logLimit) m_logMessages.pop_front();
        m_logMessages.push_back({ message, Time::GetLocalTimeStamp(), Colors::RustRed_ImGui, LogType::Error });
    }
};
