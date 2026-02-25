#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <cstddef>
#include <deque>
#include <string>
#include <imgui.h>
#include "core/utils/Colors.h"
#include "core/utils/Time.h"

enum class LogType
{
    Unknown,
    Info,
    Warning,
    Error
};

struct LogMessage
{
    std::string message;
    std::string time;
    std::string fullFilePath;
    std::string shortFilePath;
    ImVec4 color;
    LogType logType;
    int lineNumber;
};

class Debug
{
  private:

    inline static std::deque<LogMessage> m_logMessages;
    inline static constexpr size_t m_logLimit = 1000;

  public:

    inline static const std::deque<LogMessage>& GetLogMessages() { return m_logMessages; }
    inline static void Clear() { m_logMessages.clear(); }

    inline static void Log(const std::string& message,
        const std::string& file = "",
        LogType type = LogType::Unknown,
        ImVec4 color = Colors::ConcreteGrey_ImGui,
        int line = 0)
    {
        m_logMessages.push_back({ message, Time::GetLocalTimeStamp(), file, file.substr(file.find("IronHammer")), color, type, line });
    }
};

#define Log_Info(msg) Debug::Log(msg, __FILE__, LogType::Info, Colors::ConcreteGrey_ImGui, __LINE__);
#define Log_Warning(msg) Debug::Log(msg, __FILE__, LogType::Warning, Colors::HazardYellow_ImGui, __LINE__);
#define Log_Error(msg) Debug::Log(msg, __FILE__, LogType::Error, Colors::RustRed_ImGui, __LINE__);
