#pragma once

#include <string>
#include <vector>

namespace Config {

struct Paths
{
    std::string mShaders{};
    std::string mSaves{};
    std::string mGameData{};
    std::string mGraphicsOverrides{};
};

struct Graphics
{
    float mResolutionScale{4.0};
    bool mShadows{true};
    bool mEnableImGui{true};
};

struct Logging
{
    bool mLogToFile{true};
    bool mLogTime{true};
    bool mLogColours{false};
    std::string mLogFile{};
    std::string mLogLevel{"DEBUG"};
    std::vector<std::string> mDisabledLoggers{};
};

struct Config
{
    Paths mPaths{};
    Graphics mGraphics{};
    Logging mLogging{};
};

Config LoadConfig(std::string path);

}
