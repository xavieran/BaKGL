#pragma once

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

namespace Logging {

enum class LogLevel
{
    Spam,
    Debug,
    Info,
    Warn,
    Error,
    Always
};

std::string LevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Spam:   return "SPAM ";
    case LogLevel::Debug:  return "DEBUG";
    case LogLevel::Info:   return "INFO ";
    case LogLevel::Warn:   return "WARN ";
    case LogLevel::Error:  return "ERROR";
    case LogLevel::Always: return "ALWAYS";
    default: return "UNDEF";
    }
}

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

    static std::ostream& Log(LogLevel level, const std::string& loggerName)
    {
        const auto it = std::find(
            sDisabledLoggers.begin(), sDisabledLoggers.end(),
            loggerName);

        if (level >= sGlobalLogLevel && it == sDisabledLoggers.end())
            return DoLog(level, loggerName);

        return nullStream;
    }

    static const Logger& GetLogger(const std::string& name)
    {
        const auto it = std::find_if(sLoggers.begin(), sLoggers.end(),
            [&name](const auto& l){ return l->GetName() == name; });
        if (it == sLoggers.end())
        {
            return *sLoggers.emplace_back(std::make_unique<Logger>(name));
        }
        else
        {
            return **it;
        }
    }

private:
    static std::ostream& DoLog(LogLevel level, const std::string& loggerName)
    {
        const auto t = std::chrono::system_clock::now();
        const auto time = std::chrono::system_clock::to_time_t(t);
        auto gmt_time = gmtime(&time);
        auto ts = std::put_time(gmt_time, sTimeFormat.c_str());

        return std::cout << ts << " " << LevelToString(level) << " [" << loggerName << "] ";
    }

    static LogLevel sGlobalLogLevel;
    static std::string sTimeFormat;

    static std::vector<std::string> sEnabledLoggers;
    static std::vector<std::string> sDisabledLoggers;
    static std::vector<std::unique_ptr<Logger>> sLoggers;

    static std::ostream nullStream;
};

LogLevel LogState::sGlobalLogLevel{LogLevel::Info};
std::string LogState::sTimeFormat{"%H:%M:%S.%m"};

std::vector<std::string> LogState::sEnabledLoggers{};
std::vector<std::string> LogState::sDisabledLoggers{};
std::vector<std::unique_ptr<Logger>> LogState::sLoggers{};

std::ostream LogState::nullStream{nullptr};


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

std::ostream& LogDebug(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Debug, loggerName);
}

std::ostream& LogSpam(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Spam, loggerName);
}

}
