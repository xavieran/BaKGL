#include "com/logger.hpp"

#include <iostream>

namespace Logging {

std::string_view LevelToString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Spam:   return "SPAM";
    case LogLevel::Debug:  return "DEBUG";
    case LogLevel::Info:   return "INFO";
    case LogLevel::Warn:   return "WARN";
    case LogLevel::Error:  return "ERROR";
    case LogLevel::Fatal: return "FATAL";
    case LogLevel::Always: return "ALWAYS";
    default: return "UNDEF";
    }
}

std::string_view LevelToColor(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Spam:   [[fallthrough]];
    case LogLevel::Debug:  return "\033[0;37m";
    case LogLevel::Info:   return "\033[1;34m";
    case LogLevel::Warn:   return "\033[1;33m";
    case LogLevel::Error:  return "\033[1;31m";
    case LogLevel::Fatal:  [[fallthrough]];
    case LogLevel::Always: return "\033[1;4;31m"; 
    default: return "UNDEF";
    }
}

LogLevel LogState::sGlobalLogLevel{LogLevel::Info};
std::string LogState::sTimeFormat{"%H:%M:%S.%m"};
bool LogState::sLogTime{true};
bool LogState::sLogColor{false};

std::vector<std::string> LogState::sEnabledLoggers{};
std::vector<std::string> LogState::sDisabledLoggers{};
std::vector<std::unique_ptr<Logger>> LogState::sLoggers{};
OStreamMux LogState::sMux{};
std::ostream LogState::sOutput{&LogState::sMux};

std::ostream LogState::nullStream{nullptr};

std::ostream& LogFatal(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Fatal, loggerName);
}

std::ostream& LogError(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Error, loggerName);
}

std::ostream& LogWarn(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Warn, loggerName);
}

std::ostream& LogInfo(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Info, loggerName);
}

std::ostream& LogDebug(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Debug, loggerName);
}

std::ostream& LogSpam(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Spam, loggerName);
}

}
