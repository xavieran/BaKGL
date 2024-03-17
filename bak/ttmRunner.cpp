#include "bak/ttmRunner.hpp"

#include "bak/dialogSources.hpp"
#include "bak/imageStore.hpp"
#include "bak/screen.hpp"
#include "bak/textureFactory.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"

#include "graphics/types.hpp"

namespace BAK {

TTMRunner::TTMRunner(
    std::string adsFile,
    std::string ttmFile)
:
    mLogger{Logging::LogState::GetLogger("BAK::TTMRunner")}
{
    mLogger.Debug() << "Loading ADS/TTM: " << adsFile << " " << ttmFile << "\n";
    auto adsFb = BAK::FileBufferFactory::Get().CreateDataBuffer(adsFile);
    mSceneSequences = BAK::LoadSceneSequences(adsFb);
    auto ttmFb = BAK::FileBufferFactory::Get().CreateDataBuffer(ttmFile);
    mActions = BAK::LoadDynamicScenes(ttmFb);

    auto nextTag = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
    mLogger.Debug() << "Next tag: " << nextTag << "\n";
    mCurrentAction = FindActionMatchingTag(nextTag);
}

std::optional<BAK::SceneAction> TTMRunner::GetNextAction()
{
    if (mCurrentAction == mActions.size())
    {
        return std::nullopt;
    }

    auto action = mActions[mCurrentAction];
    bool nextActionChosen = false;
    std::visit(
        overloaded{
            [&](const BAK::Purge&){
                AdvanceToNextScene();
                nextActionChosen = true;
            },
            [&](const BAK::GotoTag& sa){
                mCurrentAction = FindActionMatchingTag(sa.mTag);
                nextActionChosen = true;
            },
            [&](const auto&){}
        },
        action
    );

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
        evaluate_if<BAK::SetScene>(mActions[i], [&](const auto& action) {
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
