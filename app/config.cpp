#include "app/config.hpp"

#include "com/json.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>

namespace Config {

Paths LoadPaths(const nlohmann::json& config)
{
    Paths paths{};
    if (config.contains("Paths"))
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
        logging.mLogFilePath = c.value("LogFilePath", "");
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

Audio LoadAudio(const nlohmann::json& config)
{
    Audio audio{};
    if (config.contains("Audio"))
    {
        const auto& c = config["Audio"];
        audio.mEnableAudio = c.value("EnableAudio", true);
        audio.mEnableBackgroundSounds = c.value("EnableBackgroundSounds", true);
        audio.mMidiPlayer = c.value("MidiPlayer", "ADLMIDI");
    }
    return audio;
}

Game LoadGame(const nlohmann::json& config)
{
    Game game{};
    if (config.contains("Game"))
    {
        const auto& c = config["Game"];
        game.mAdvanceTime = c.value("AdvanceTime", true);
    }
    return game;
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
    config.mAudio = LoadAudio(data);
    config.mGame = LoadGame(data);

    std::cout << "Loaded config file: " << data <<"\n";

    return config;
}

}
