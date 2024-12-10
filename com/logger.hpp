#pragma once

#include "com/ostreamMux.hpp"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Logging {

enum class LogLevel
{
    Spam,
    Debug,
    Info,
    Warn,
    Error,
    Fatal,
    Always
};

std::string_view LevelToString(LogLevel level);
std::string_view LevelToColor(LogLevel level);

class Logger;

class LogState
{
public:
    static void Disable(const std::string& logger)
    {
        sDisabledLoggers.emplace_back(logger);
    }

    static void SetLevel(LogLevel level)
    {
        sGlobalLogLevel = level;
    }

    static void SetLevel(std::string_view level)
    {
        if (level == LevelToString(Logging::LogLevel::Spam))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Spam);
        }
        else if (level == LevelToString(Logging::LogLevel::Debug))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Debug);
        }
        else if (level == LevelToString(Logging::LogLevel::Info))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Info);
        }
        else if (level == LevelToString(Logging::LogLevel::Warn))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Warn);
        }
        else if (level == LevelToString(Logging::LogLevel::Fatal))
        {
            Logging::LogState::SetLevel(Logging::LogLevel::Fatal);
        }
        else
        {
            throw std::runtime_error("Invalid log level: " + std::string(level));
        }
    }

    static void SetLogTime(bool value)
    {
        sLogTime = value;
    }

    static void SetLogColor(bool value)
    {
        sLogColor = value;
    }

    static std::ostream& Log(LogLevel level, const std::string& loggerName)
    {
        const auto it = std::find(
            sDisabledLoggers.begin(), sDisabledLoggers.end(),
            loggerName);

        if (level >= sGlobalLogLevel && it == sDisabledLoggers.end())
            return DoLog(level, loggerName);

        return nullStream;
    }
    
    static const Logger& GetLogger(const std::string& name){ return GetLoggerT<Logger>(name); }
    
    // FIXME: Clang complains about incomplete type in lambda
    template <typename T>
    static const T& GetLoggerT(const std::string& name)
    {
        const auto it = std::find_if(sLoggers.begin(), sLoggers.end(),
            [&name](const auto& l){ return l->GetName() == name; });
        if (it == sLoggers.end())
        {
            return *sLoggers.emplace_back(std::make_unique<T>(name));
        }
        else
        {
            return **it;
        }
    }

    static void AddStream(std::ostream* stream)
    {
        sMux.AddStream(stream);
    }

    static void RemoveStream(std::ostream* stream)
    {
        sMux.RemoveStream(stream);
    }

private:
    static std::ostream& DoLog(LogLevel level, const std::string& loggerName)
    {
        if (sLogTime)
        {
            const auto t = std::chrono::system_clock::now();
            const auto time = std::chrono::system_clock::to_time_t(t);
            auto gmt_time = tm{};
#ifdef _MSC_VER
            gmtime_s(&gmt_time , &time);
#else
            gmtime_r(&time, &gmt_time);
#endif
            sOutput << std::put_time(&gmt_time, sTimeFormat.c_str()) << " ";
        }
        if (sLogColor)
        {
            sOutput << LevelToColor(level);
        }

        sOutput << LevelToString(level) << " [" << loggerName << "] ";
        if (sLogColor)
        {
             sOutput << "\033[0m ";
        }

        return sOutput;
    }

    static LogLevel sGlobalLogLevel;
    static std::string sTimeFormat;
    static bool sLogTime;
    static bool sLogColor;

    static std::vector<std::string> sEnabledLoggers;
    static std::vector<std::string> sDisabledLoggers;
    static std::vector<std::unique_ptr<Logger>> sLoggers;
    static OStreamMux sMux;
    static std::ostream sOutput;

    static std::ostream nullStream;
};


class Logger
{
public:
    Logger(std::string name)
    :
        mName{name}
    {
    }

    std::ostream& Debug() const
    {
        return LogState::Log(LogLevel::Debug, mName);
    }
    
    template <typename T>
    void Debug(T&& log) const
    {
        Debug() << std::forward<T>(log) << std::endl;
    }

    std::ostream& Info() const
    {
        return LogState::Log(LogLevel::Info, mName);
    }

    std::ostream& Warn() const
    {
        return LogState::Log(LogLevel::Warn, mName);
    }

    std::ostream& Error() const
    {
        return LogState::Log(LogLevel::Error, mName);
    }

    std::ostream& Spam() const
    {
        return LogState::Log(LogLevel::Spam, mName);
    }

    std::ostream& Log(LogLevel level) const
    {
        return LogState::Log(level, mName);
    }

    const std::string& GetName() const { return mName; }

private:

    std::string mName;
};

std::ostream& LogFatal(const std::string& loggerName);
std::ostream& LogError(const std::string& loggerName);
std::ostream& LogWarn(const std::string& loggerName);
std::ostream& LogInfo(const std::string& loggerName);
std::ostream& LogDebug(const std::string& loggerName);
std::ostream& LogSpam(const std::string& loggerName);

}
