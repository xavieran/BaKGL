#pragma once

#include "bak/scene/ads.hpp"
#include "bak/scene/sceneData.hpp"

#include "com/logger.hpp"

#include <deque>
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

    std::optional<ScriptAction> GetNextAction();

private:
    void StartNextScript();
    bool EvaluateScene();
    void ExecuteBlock(const ADS::ActionBlock& block);
    bool EvaluateCondition(const ADS::Condition& condition) const;
    void StartScript(unsigned scriptTag);
    void StopScript(unsigned scriptTag);
    unsigned FindActionMatchingTag(unsigned tag) const;

    ADS::Ads mAds;
    std::vector<ScriptAction> mActions;

    std::unordered_set<unsigned> mStarted;
    std::unordered_set<unsigned> mFinished;
    std::deque<unsigned> mPendingScripts;
    unsigned mCurrentScript = 0;

    unsigned mCurrentAction = 0;

    const Logging::Logger& mLogger;
};

}
