#include "bak/scene/ttmRunner.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/scene/scene.hpp"

#include "com/assert.hpp"
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
    mAds = ADS::LoadAds(adsFb);
    auto ttmFb = FileBufferFactory::Get().CreateDataBuffer(ttmFile);
    mFrames = LoadDynamicScripts(ttmFb);

    mStarted.clear();
    mFinished.clear();
    mPendingScripts.clear();
    mCurrentScript = 0;
    mCurrentFrame.reset();

    EvaluateScene();
    StartNextScript();
}

std::optional<ScriptFrame> TTMRunner::GetNextFrame()
{
    while (mCurrentFrame)
    {
        if (*mCurrentFrame >= mFrames.size())
        {
            FinishCurrentScript();
            continue;
        }

        const auto frame = mFrames[*mCurrentFrame];

        bool endScript = false;
        std::optional<unsigned> gotoTag{};
        for (const auto& action : frame.mActions)
        {
            std::visit(
                overloaded{
                    [&](const EndScript&){ endScript = true; },
                    [&](const GotoTag& sa){ gotoTag = sa.mTag; },
                    [&](const auto&){}
                },
                action
            );
        }

        if (endScript)
        {
            FinishCurrentScript();
        }
        else if (gotoTag)
        {
            mCurrentFrame = FindFrameMatchingTag(*gotoTag);
            if (!mCurrentFrame)
            {
                mLogger.Debug() << "No frame tagged: " << *gotoTag << "\n";
                FinishCurrentScript();
            }
        }
        else
        {
            mCurrentFrame = *mCurrentFrame + 1;
        }

        return frame;
    }

    return std::nullopt;
}

void TTMRunner::FinishCurrentScript()
{
    mLogger.Debug() << "Finished script: " << mCurrentScript << "\n";
    mFinished.insert(mCurrentScript);
    EvaluateScene();
    StartNextScript();
}

void TTMRunner::StartNextScript()
{
    while (!mPendingScripts.empty())
    {
        mCurrentScript = mPendingScripts.front();
        mPendingScripts.pop_front();
        mCurrentFrame = FindFrameMatchingTag(mCurrentScript);
        if (mCurrentFrame)
        {
            mLogger.Debug() << "Starting script: " << mCurrentScript << "\n";
            return;
        }

        mLogger.Debug() << "No frame tagged: " << mCurrentScript << "\n";
    }

    mCurrentFrame.reset();
}

bool TTMRunner::EvaluateScene()
{
    for (const auto& scene : mAds.mScenes)
    {
        for (const auto& block : scene.mBlocks)
        {
            bool conditionsHold = true;
            for (const auto& condition : block.mConditions)
            {
                conditionsHold &= EvaluateCondition(condition);
            }

            if (conditionsHold)
            {
                mLogger.Debug() << "Block matched: " << block << "\n";
                ExecuteBlock(block);
                return true;
            }
        }
    }

    return false;
}

void TTMRunner::ExecuteBlock(const ADS::ActionBlock& block)
{
    for (const auto& action : block.mThen)
    {
        std::visit(
            overloaded{
                [&](const ADS::StartScript& start){
                    StartScript(start.mScriptIndex.mValue);
                },
                [&](const ADS::StopScript& stop){
                    StopScript(stop.mScriptIndex.mValue);
                }},
            action);
    }
}

bool TTMRunner::EvaluateCondition(const ADS::Condition& condition) const
{
    return std::visit(
        overloaded{
            [&](const ADS::NotStarted& notStarted){
                return !mStarted.contains(notStarted.mScriptIndex.mValue);
            },
            [&](const ADS::Finished& finished){
                return mFinished.contains(finished.mScriptIndex.mValue);
            },
            [&](const ADS::ChapterGTE&){
                ASSERT(false);
                return false;
            },
            [&](const ADS::ChapterLTE&){
                ASSERT(false);
                return false;
            }},
        condition);
}

void TTMRunner::StartScript(unsigned scriptTag)
{
    mStarted.insert(scriptTag);
    mPendingScripts.emplace_back(scriptTag);
}

void TTMRunner::StopScript(unsigned scriptTag)
{
    std::erase(mPendingScripts, scriptTag);
}

std::optional<unsigned> TTMRunner::FindFrameMatchingTag(unsigned tag) const
{
    for (unsigned i = 0; i < mFrames.size(); i++)
    {
        if (mFrames[i].mTag == tag)
        {
            return i;
        }
    }

    return std::nullopt;
}

}
