#include "app/config.hpp"

#include "com/json.hpp"

#include <filesystem>
#include <fstream>

namespace Config {

Paths LoadPaths(const nlohmann::json& config)
{
    Paths paths{};
    if (config.contains("Path"))
    {
        const auto& c = config["Paths"];
        paths.mShaders = c.value("Shaders", "");
        paths.mSaves = c.value("Saves", "");
        paths.mGameData = c.value("GameData", "");
        paths.mGraphicsOverrides = c.value("GraphicsOverrides", "");
    }
    return paths;
}

Graphics LoadGraphics(const nlohmann::json& config)
{
    Graphics graphics{};
    if (config.contains("Graphics"))
    {
        const auto& c = config["Graphics"];
        graphics.mResolutionScale = c.value("ResolutionScale", 4.0);
        graphics.mShadows = c.value("EnableShadows", true);
        graphics.mEnableImGui = c.value("EnableImGui", true);
    }
    return graphics;
}

Logging LoadLogging(const nlohmann::json& config)
{
    Logging logging{};
    if (config.contains("Logging"))
    {
        const auto& c = config["Logging"];
        logging.mLogToFile = c.value("LogToFile", true);
        logging.mLogFile = c.value("LogFile", "");
        logging.mLogLevel = c.value("LogLevel", "Debug");
        if (c.contains("DisabledLoggers"))
        {
            for (const auto& logger : c["DisabledLoggers"])
            {
                logging.mDisabledLoggers.emplace_back(logger);
            }
        }
    }
    return logging;
}

Config LoadConfig(std::string path)
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("No config file at path: " + path);
    }

    std::ifstream in{};
    in.open(path, std::ios::in);
    auto data = nlohmann::json::parse(in);

    Config config{};
    config.mPaths = LoadPaths(data);
    config.mGraphics = LoadGraphics(data);
    config.mLogging = LoadLogging(data);

    return config;
}

}
