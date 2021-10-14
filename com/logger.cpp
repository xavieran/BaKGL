#include "com/logger.hpp"

namespace Logging {

std::string_view LevelToString(LogLevel level)
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

LogLevel LogState::sGlobalLogLevel{LogLevel::Info};
std::string LogState::sTimeFormat{"%H:%M:%S.%m"};

std::vector<std::string> LogState::sEnabledLoggers{};
std::vector<std::string> LogState::sDisabledLoggers{};
std::vector<std::unique_ptr<Logger>> LogState::sLoggers{};
OStreamMux LogState::sMux{};
std::ostream LogState::sOutput{&LogState::sMux};

std::ostream LogState::nullStream{nullptr};


std::ostream& LogDebug(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Debug, loggerName);
}

std::ostream& LogSpam(const std::string& loggerName)
{
    return LogState::Log(Logging::LogLevel::Spam, loggerName);
}

}
