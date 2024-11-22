#include "bak/ttmRunner.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/scene.hpp"

#include "com/logger.hpp"
#include "com/visit.hpp"

namespace BAK {

TTMRunner::TTMRunner()
:
    mLogger{Logging::LogState::GetLogger("BAK::TTMRunner")}
{
}

void TTMRunner::LoadTTM(
    std::string adsFile,
    std::string ttmFile)
{
    mLogger.Debug() << "Loading ADS/TTM: " << adsFile << " " << ttmFile << "\n";
    auto adsFb = FileBufferFactory::Get().CreateDataBuffer(adsFile);
    mSceneSequences = LoadSceneSequences(adsFb);
    auto ttmFb = FileBufferFactory::Get().CreateDataBuffer(ttmFile);
    mActions = LoadDynamicScenes(ttmFb);

    mCurrentSequence = 0;
    mCurrentSequenceScene = 0;
    auto nextTag = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
    mLogger.Debug() << "Next tag: " << nextTag << "\n";
    mCurrentAction = FindActionMatchingTag(nextTag);
}

std::optional<SceneAction> TTMRunner::GetNextAction()
{
    if (mCurrentAction == mActions.size())
    {
        return std::nullopt;
    }

    auto action = mActions[mCurrentAction];
    bool nextActionChosen = false;
    bool finishEarly = false;

    std::visit(
        overloaded{
            [&](const Purge&){
                AdvanceToNextScene();
                nextActionChosen = true;
            },
            [&](const GotoTag& sa){
                // Hack til I figure out exactly how C31 works...
                if (sa.mTag == 4)
                {
                    finishEarly = true;
                    return;
                }
                mCurrentAction = FindActionMatchingTag(sa.mTag);
                nextActionChosen = true;
            },
            [&](const auto&){}
        },
        action
    );

    if (finishEarly)
    {
        return std::nullopt;
    }

    if (nextActionChosen)
    {
        if (mCurrentAction == mActions.size())
        {
            return std::nullopt;
        }

        action = mActions[mCurrentAction];
    }

    mCurrentAction++;

    return action;
}

void TTMRunner::AdvanceToNextScene()
{
    auto& currentScenes = mSceneSequences[1][mCurrentSequence].mScenes;
    mCurrentSequenceScene++;
    if (mCurrentSequenceScene == currentScenes.size())
    {
        mCurrentSequenceScene = 0;
        mCurrentSequence++;
    }

    if (mCurrentSequence == mSceneSequences[1].size())
    {
        mCurrentAction = mActions.size();
        mCurrentSequence = 0;
        return;
    }

    auto nextTag = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
    mCurrentAction = FindActionMatchingTag(nextTag);
}

unsigned TTMRunner::FindActionMatchingTag(unsigned tag)
{
    std::optional<unsigned> foundIndex{};
    for (unsigned i = 0; i < mActions.size(); i++)
    {
        evaluate_if<SetScene>(mActions[i], [&](const auto& action) {
            if (action.mSceneNumber == tag)
            {
                foundIndex = i;
            }
        });
        if (foundIndex)
        {
            return *foundIndex;
        }
    }

    return 0;
}

}
