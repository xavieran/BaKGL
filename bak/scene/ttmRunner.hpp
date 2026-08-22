#pragma once

#include "bak/scene/ads.hpp"
#include "bak/scene/sceneData.hpp"

#include "com/logger.hpp"

#include <optional>
#include <unordered_set>
#include <vector>

namespace BAK {

class TTMRunner
{
    
public:
    TTMRunner();

    void LoadTTM(
        std::string adsFile,
        std::string ttmFile);

    std::optional<ScriptFrame> GetNextFrame();

private:
    struct RunningScript
    {
        unsigned mTag;
        unsigned mFrame;
        bool mRunning;
    };

    bool StepScript(RunningScript& script, std::vector<ScriptAction>& actions);
    bool EvaluateScene();
    void ExecuteBlock(const ADS::ActionBlock& block);
    bool EvaluateCondition(const ADS::Condition& condition) const;
    void StartScript(unsigned scriptTag);
    void StopScript(unsigned scriptTag);
    std::optional<unsigned> FindFrameMatchingTag(unsigned tag) const;

    ADS::Ads mAds;
    std::vector<ScriptFrame> mFrames;

    std::unordered_set<unsigned> mStarted;
    std::unordered_set<unsigned> mFinished;
    std::vector<RunningScript> mRunningScripts;

    const Logging::Logger& mLogger;
};

}
