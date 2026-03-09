#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <cstddef>
#include <deque>
#include <string>
#include <imgui.h>
#include <backward.hpp>
#include <regex>
#include <vector>
#include "core/utils/Colors.h"
#include "core/utils/Time.h"

enum class LogType
{
    Unknown,
    Info,
    Warning,
    Error
};

struct TraceBreakdown
{
    std::string objectFunction;
    std::string path;
    unsigned int line;
    unsigned int column;

    TraceBreakdown(const std::string& func, const std::string& file, unsigned int line, unsigned int col)
        : objectFunction(func), path(file), line(line), column(col)
    {
    }

    std::string ToString() const { return objectFunction + " at " + path + " line " + std::to_string(line); }
};

struct LogMessage
{
    std::string message;
    std::string time;
    std::vector<TraceBreakdown> traceBreakdowns;
    ImVec4 color;
    LogType logType;
};

class Debug
{
  private:

    inline static std::deque<LogMessage> m_logMessages;
    inline static constexpr size_t m_logLimit = 1000;

    inline static size_t m_infoLogCount = 0;
    inline static size_t m_warningLogCount = 0;
    inline static size_t m_errorLogCount = 0;

    inline static std::string SimplifyObjectFunctionName(const std::string& func)
    {
        std::string simplified = func;

        // Remove std::__cxx11:: prefix
        simplified = std::regex_replace(simplified, std::regex("std::__cxx11::"), "");
        // Remove template arguments <> (simple approach)
        simplified = std::regex_replace(simplified, std::regex("<.*>"), "");

        return simplified;
    }

    inline static std::vector<TraceBreakdown> GetTraceBreakdown(size_t skipFramesFromStart = 4, size_t skipFramesFromEnd = 4)
    {
        backward::StackTrace stackTrace;
        stackTrace.load_here(32);
        backward::TraceResolver traceResolver;
        traceResolver.load_stacktrace(stackTrace);

        std::vector<TraceBreakdown> traceBreakdowns;
        traceBreakdowns.reserve(stackTrace.size() - skipFramesFromEnd - skipFramesFromStart);

        for (size_t i = skipFramesFromStart; i < stackTrace.size() - skipFramesFromEnd; ++i)
        {
            const backward::ResolvedTrace& resolvedTrace = traceResolver.resolve(stackTrace[i]);
            traceBreakdowns.emplace_back(SimplifyObjectFunctionName(resolvedTrace.object_function),
                resolvedTrace.source.filename,
                resolvedTrace.source.line,
                resolvedTrace.source.col);
        }
        return std::move(traceBreakdowns);
    }

  public:

    inline static const std::deque<LogMessage>& GetLogMessages() { return m_logMessages; }
    inline static void Clear()
    {
        m_infoLogCount = 0;
        m_warningLogCount = 0;
        m_errorLogCount = 0;
        m_logMessages.clear();
    }

    inline static size_t GetInfoLogCount() { return m_infoLogCount; }
    inline static size_t GetWarningLogCount() { return m_warningLogCount; }
    inline static size_t GetErrorLogCount() { return m_errorLogCount; }

    inline static void Log(const std::string& message, LogType type = LogType::Unknown, ImVec4 color = Colors::ConcreteGrey_ImGui)
    {
        m_logMessages.push_back({ message, Time::GetLocalTimeStamp(), GetTraceBreakdown(4,4), color, type });

        switch (type)
        {
            case LogType::Info: ++m_infoLogCount; break;
            case LogType::Warning: ++m_warningLogCount; break;
            case LogType::Error: ++m_errorLogCount; break;
            case LogType::Unknown:; break;
        }
    }
};

#define LOG_INFO(msg) Debug::Log(msg, LogType::Info, Colors::ConcreteGrey_ImGui);
#define LOG_WARNING(msg) Debug::Log(msg, LogType::Warning, Colors::HazardYellow_ImGui);
#define LOG_ERROR(msg) Debug::Log(msg, LogType::Error, Colors::RustRed_ImGui);
