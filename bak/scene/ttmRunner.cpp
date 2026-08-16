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
    mActions = LoadDynamicScripts(ttmFb);

    mStarted.clear();
    mFinished.clear();
    mPendingScripts.clear();
    mCurrentScript = 0;
    mCurrentAction = 0;

    StartNextScript();
}

std::optional<ScriptAction> TTMRunner::GetNextAction()
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
                StopScript(mCurrentScript);
                StartNextScript();
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

void TTMRunner::StartNextScript()
{
    if (mPendingScripts.empty())
    {
        EvaluateScene();
    }

    if (mPendingScripts.empty())
    {
        mCurrentAction = mActions.size();
        return;
    }

    mCurrentScript = mPendingScripts.front();
    mPendingScripts.pop_front();
    mLogger.Debug() << "Starting script: " << mCurrentScript << "\n";
    mCurrentAction = FindActionMatchingTag(mCurrentScript);
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
    if (scriptTag == mCurrentScript)
    {
        mFinished.insert(scriptTag);
    }
}

unsigned TTMRunner::FindActionMatchingTag(unsigned tag) const
{
    std::optional<unsigned> foundIndex{};
    for (unsigned i = 0; i < mActions.size(); i++)
    {
        evaluate_if<SetScript>(mActions[i], [&](const auto& action) {
            if (action.mScriptNumber == tag)
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
