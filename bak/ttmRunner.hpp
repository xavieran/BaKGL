#pragma once

#include "bak/scene.hpp"

#include "com/logger.hpp"

namespace BAK {

class TTMRunner
{
    
public:
    TTMRunner();

    void LoadTTM(
        std::string adsFile,
        std::string ttmFile);

    std::optional<BAK::SceneAction> GetNextAction();

private:
    void AdvanceToNextScene();
    unsigned FindActionMatchingTag(unsigned tag);

    std::unordered_map<unsigned, std::vector<BAK::SceneSequence>> mSceneSequences;
    std::vector<BAK::SceneAction> mActions;

    unsigned mCurrentAction = 0;
    unsigned mCurrentSequence = 0;
    unsigned mCurrentSequenceScene = 0;

    const Logging::Logger& mLogger;
};

}
