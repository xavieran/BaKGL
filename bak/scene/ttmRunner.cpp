#include "bak/scene/ttmRunner.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/scene/scene.hpp"

#include "com/assert.hpp"
#include "com/logger.hpp"
#include "com/visit.hpp"

#include <algorithm>

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
    mRunningScripts.clear();
}

std::optional<ScriptFrame> TTMRunner::GetNextFrame()
{
    while (true)
    {
        EvaluateScene();

        if (mRunningScripts.empty())
        {
            return std::nullopt;
        }

        ScriptFrame frame{};
        bool stepped = false;
        for (auto& script : mRunningScripts)
        {
            stepped |= StepScript(script, frame.mActions);
        }

        for (const auto& script : mRunningScripts)
        {
            if (!script.mRunning)
            {
                mLogger.Debug() << "Finished script: " << script.mTag << "\n";
                mFinished.insert(script.mTag);
            }
        }
        std::erase_if(mRunningScripts, [](const auto& s){ return !s.mRunning; });

        if (stepped)
        {
            return frame;
        }
    }
}

bool TTMRunner::StepScript(RunningScript& script, std::vector<ScriptAction>& actions)
{
    if (script.mFrame >= mFrames.size())
    {
        script.mRunning = false;
        return false;
    }

    const auto& frame = mFrames[script.mFrame];

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
        actions.emplace_back(action);
    }

    if (endScript)
    {
        script.mRunning = false;
    }
    else if (gotoTag)
    {
        const auto taggedFrame = FindFrameMatchingTag(*gotoTag);
        if (taggedFrame)
        {
            script.mFrame = *taggedFrame;
        }
        else
        {
            mLogger.Debug() << "No frame tagged: " << *gotoTag << "\n";
            script.mRunning = false;
        }
    }
    else
    {
        script.mFrame++;
    }

    return true;
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

    const auto startFrame = FindFrameMatchingTag(scriptTag);
    if (!startFrame)
    {
        mLogger.Debug() << "No frame tagged: " << scriptTag << "\n";
        return;
    }

    const auto alreadyRunning = std::any_of(
        mRunningScripts.begin(), mRunningScripts.end(),
        [scriptTag](const auto& s){ return s.mTag == scriptTag; });
    if (alreadyRunning)
    {
        return;
    }

    mLogger.Debug() << "Starting script: " << scriptTag << "\n";
    mRunningScripts.emplace_back(scriptTag, *startFrame, true);
}

void TTMRunner::StopScript(unsigned scriptTag)
{
    mLogger.Debug() << "Stopping script: " << scriptTag << "\n";
    std::erase_if(
        mRunningScripts,
        [scriptTag](const auto& s){ return s.mTag == scriptTag; });
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
