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
}

Graphics::TextureStore TTMRunner::RenderTTM()
{

    mLogger.Debug() << "SceneSequences\n";
    for (const auto& [key, sequences] : mSceneSequences)
    {
        mLogger.Debug() << "Key: " << key << "\n";
        for (const auto& sequence : sequences)
        {
            mLogger.Debug() << "  Sequence: " << sequence.mName << "\n";
            for (const auto& scene : sequence.mScenes)
            {
                mLogger.Debug() << "    ADS(" << scene.mInitScene << ", " << scene.mDrawScene << ")\n";
            }
        }
    }

    auto nextTag = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
    mLogger.Debug() << "Next tag: " << nextTag << "\n";
    mCurrentAction = FindActionMatchingTag(nextTag);
    mLogger.Debug() << "Current action: " << mCurrentAction << "\n";
    while (!AdvanceAction())
    {
    }
    return mRenderedFrames;
}

bool TTMRunner::AdvanceAction()
{
    mLogger.Debug() << "AdvanceAction" << "\n";
    const auto& action = mActions[mCurrentAction];
    bool nextActionChosen = false;
    mLogger.Debug() << "Handle action: " << action << std::endl;
    std::visit(
        overloaded{
            [&](const BAK::Purge&){
                AdvanceToNextScene();
                nextActionChosen = true;
            },
            [&](const BAK::GotoTag& sa){
                mCurrentAction = FindActionMatchingTag(sa.mTag);
                nextActionChosen = true;;
            },
            [&](const auto&){}
        },
        action
    );

    if (!nextActionChosen)
    {
        mCurrentAction++;
        if (mCurrentAction == mActions.size())
        {
            return true;
        }
    }
    else if (mCurrentAction == mActions.size())
    {
        return true;
    }

    return false;
}

void TTMRunner::AdvanceToNextScene()
{
    auto& currentScenes = mSceneSequences[1][mCurrentSequence].mScenes;
    mCurrentSequenceScene++;
    if (mCurrentSequenceScene == currentScenes.size())
    {
        mLogger.Info() << "Finished current scene sequence, moving to next sequence\n";
        mCurrentSequenceScene = 0;
        mCurrentSequence++;
    }

    if (mCurrentSequence == mSceneSequences[1].size())
    {
        mLogger.Info() << "Finished all sequences, current action: "
            << mCurrentAction << " actions size: " << mActions.size() << "\n";
        mCurrentAction = mActions.size();
        mCurrentSequence = 0;
        return;
    }

    auto nextTag = mSceneSequences[1][mCurrentSequence].mScenes[mCurrentSequenceScene].mDrawScene;
    mLogger.Debug() << "Next tag: " << nextTag << "\n";
    mCurrentAction = FindActionMatchingTag(nextTag);
    mLogger.Debug() << "Current action: " << mCurrentAction << "\n";
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
