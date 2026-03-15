#pragma once
#include <SFML/Window/Keyboard.hpp>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <queue>
#include <string>
#include <imgui.h>
#include <backward.hpp>
#include <regex>
#include <thread>
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
    std::string result;

    TraceBreakdown(const std::string& func, const std::string& file, unsigned int line, unsigned int col)
        : objectFunction(func), path(file), line(line), column(col), result(objectFunction + " at " + path + " line " + std::to_string(line))
    {
    }
};

struct PendingLogMessage
{
    std::string message;
    std::string time;
    ImVec4 color;
    LogType logType;
    backward::StackTrace stackTrace;

    PendingLogMessage(const std::string& message, std::string time, ImVec4 color, LogType logType, backward::StackTrace&& trace)
        : message(message), time(std::move(time)), color(color), logType(logType), stackTrace(std::move(trace))
    {
    }
};

struct LogMessage
{
    std::string message;
    std::string time;
    std::string displayLable;
    std::vector<TraceBreakdown> traceBreakdowns;
    ImVec4 color;
    LogType logType;

    LogMessage(std::string message, std::string time, std::vector<TraceBreakdown> traceBreakdowns, ImVec4 color, LogType logType)
        : message(std::move(message))
        , time(std::move(time))
        , displayLable(this->time + " " + this->message)
        , traceBreakdowns(std::move(traceBreakdowns))
        , color(color)
        , logType(logType)
    {
    }
};

struct LogCounts
{
    size_t infoLogCount = 0;
    size_t warningLogCount = 0;
    size_t errorLogCount = 0;
};

class Debug
{
  private:

    inline static std::deque<LogMessage> m_logMessageQueue;
    inline static std::deque<LogMessage> m_logMessageQueueBuffer;
    inline static std::queue<PendingLogMessage> m_pendingLogMesssageQueue;
    inline static constexpr size_t m_logLimit = 1000;

    inline static std::thread m_loggerThread;
    inline static std::mutex m_pendingLogsMutex;
    inline static std::mutex m_logMessagesMutex;
    inline static std::condition_variable m_cv;
    inline static std::atomic<bool> m_loggerRunning = true;

    inline static LogCounts m_logCounts;
    inline static LogCounts m_logCountsBuffer;

    inline static std::string SimplifyObjectFunctionName(const std::string& func)
    {
        static const std::regex cxx11Pattern("std::__cxx11::");
        static const std::regex templatePattern("<.*>");

        std::string simplified = std::regex_replace(func, cxx11Pattern, "");
        return std::regex_replace(simplified, templatePattern, "");
    }

    inline static std::vector<TraceBreakdown> ResolveStackTrace(
        backward::StackTrace& stackTrace, size_t skipFramesFromStart = 4, size_t skipFramesFromEnd = 4)
    {
        std::vector<TraceBreakdown> traceBreakdowns;

        if (stackTrace.size() <= skipFramesFromEnd + skipFramesFromStart) return traceBreakdowns;

        backward::TraceResolver traceResolver;
        traceResolver.load_stacktrace(stackTrace);

        traceBreakdowns.reserve(stackTrace.size() - skipFramesFromEnd - skipFramesFromStart);

        for (size_t i = skipFramesFromStart; i < stackTrace.size() - skipFramesFromEnd; ++i)
        {
            const backward::ResolvedTrace& resolvedTrace = traceResolver.resolve(stackTrace[i]);
            traceBreakdowns.emplace_back(SimplifyObjectFunctionName(resolvedTrace.object_function),
                resolvedTrace.source.filename,
                resolvedTrace.source.line,
                resolvedTrace.source.col);
        }
        return traceBreakdowns;
    }

  public:

    inline static const std::deque<LogMessage>& GetLogMessages() { return m_logMessageQueue; }
    inline static LogCounts& GetLogCounts() { return m_logCounts; }

    inline static void StartLoggerThread()
    {
        m_loggerThread = std::thread(
            []()
            {
                while (true)
                {
                    std::unique_lock pendingLogsLock(m_pendingLogsMutex);
                    m_cv.wait(pendingLogsLock, [] { return !m_pendingLogMesssageQueue.empty() || !m_loggerRunning; });
                    if (!m_loggerRunning && m_pendingLogMesssageQueue.empty()) break;

                    while (!m_pendingLogMesssageQueue.empty())
                    {
                        PendingLogMessage pendingLog = std::move(m_pendingLogMesssageQueue.front());
                        m_pendingLogMesssageQueue.pop();
                        pendingLogsLock.unlock();

                        std::vector<TraceBreakdown> breakdowns = ResolveStackTrace(pendingLog.stackTrace);
                        {
                            std::lock_guard logMessagesLock(m_logMessagesMutex);
                            m_logMessageQueueBuffer.emplace_back(
                                pendingLog.message, pendingLog.time, breakdowns, pendingLog.color, pendingLog.logType);

                            switch (pendingLog.logType)
                            {
                                case LogType::Info: ++m_logCountsBuffer.infoLogCount; break;
                                case LogType::Warning: ++m_logCountsBuffer.warningLogCount; break;
                                case LogType::Error: ++m_logCountsBuffer.errorLogCount; break;
                                case LogType::Unknown:; break;
                            }
                        }
                        pendingLogsLock.lock();
                    }
                }
            });
    }

    // Very Important
    //  Called at the end, and outside the main loop
    inline static void ShutdownLoggerThread()
    {
        m_loggerRunning = false;
        m_cv.notify_one();
        if (m_loggerThread.joinable()) m_loggerThread.join();
    }

    // Very Important
    // Called at the end of each frame in the main loop
    inline static void FlushLogs()
    {
        std::lock_guard lock(m_logMessagesMutex);
        for (auto& logMessage : m_logMessageQueueBuffer)
        {
            m_logMessageQueue.emplace_back(std::move(logMessage));
        }
        m_logMessageQueueBuffer.clear();
        m_logCounts = m_logCountsBuffer;
    }

    inline static void Clear()
    {
        m_logCounts = {};
        m_logMessageQueue.clear();
        m_logCountsBuffer = {};
    }

    inline static void Log(const std::string& message, ImVec4 color = Colors::ConcreteGrey_ImGui, LogType logType = LogType::Unknown)
    {
        if (m_logMessageQueue.size() > m_logLimit)
        {
            std::lock_guard lock(m_logMessagesMutex);
            m_logMessageQueue.pop_front();
        }

        backward::StackTrace stackTrace;

        switch (logType)
        {
            case LogType::Info: stackTrace.load_here(0); break;
            case LogType::Error: stackTrace.load_here(32); break;
            case LogType::Warning: stackTrace.load_here(16); break;
            case LogType::Unknown: stackTrace.load_here(0); break;
        }
        {
            std::lock_guard lock(m_pendingLogsMutex);
            m_pendingLogMesssageQueue.push({ message, Time::GetLocalTimeStamp(), color, logType, std::move(stackTrace) });
        }
        m_cv.notify_one();
    }
};

#define LOG_INFO(msg) Debug::Log(msg, Colors::ConcreteGrey_ImGui, LogType::Info);
#define LOG_WARNING(msg) Debug::Log(msg, Colors::HazardYellow_ImGui, LogType::Warning);
#define LOG_ERROR(msg) Debug::Log(msg, Colors::RustRed_ImGui, LogType::Error);
