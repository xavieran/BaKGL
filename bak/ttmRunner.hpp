#pragma once

#include "bak/sceneData.hpp"

#include "com/logger.hpp"

#include <unordered_map>

namespace BAK {

struct SceneSequence;

class TTMRunner
{
    
public:
    TTMRunner();

    void LoadTTM(
        std::string adsFile,
        std::string ttmFile);

    std::optional<SceneAction> GetNextAction();

private:
    void AdvanceToNextScene();
    unsigned FindActionMatchingTag(unsigned tag);

    std::unordered_map<unsigned, std::vector<SceneSequence>> mSceneSequences;
    std::vector<SceneAction> mActions;

    unsigned mCurrentAction = 0;
    unsigned mCurrentSequence = 0;
    unsigned mCurrentSequenceScene = 0;

    const Logging::Logger& mLogger;
};

}
