#include "gui/dialogRunner.hpp"

#include "audio/audio.hpp"

namespace Gui {

DialogRunner::DialogRunner(
    glm::vec2 pos,
    glm::vec2 dims,
    const Actors& actors,
    const Backgrounds& bgs,
    const Font& fr,
    BAK::GameState& gameState,
    ScreenStack& screenStack,
    FinishCallback&& finished)
:
    Widget{
        RectTag{},
        pos,
        dims,
        glm::vec4{0},
        true
    },
    mScreenStack{screenStack},
    mDialogState{false, false, glm::vec2{0}, glm::vec2{0}},
    mChoices{
        pos,
        dims,
        fr,
        screenStack,
        [this](auto choice){
            MakeChoice(choice);
        }
    },
    mKeywords{},
    mGameState{gameState},
    mCenter{160, 112},
    mFont{fr},
    mActors{actors},
    mCurrentTarget{},
    mLastChoice{},
    mPendingZoneTeleport{},
    mTargetStack{},
    mStartedMusic{false},
    mRemainingText{""},
    mTextDims{0},
    mDialogDisplay{pos, dims, actors, bgs, fr, gameState},
    mFinished{finished},
    mDialogScene{nullptr},
    mLogger{Logging::LogState::GetLogger("Gui::DialogRunner")}
{
    AddChildBack(&mDialogDisplay);
    ASSERT(mFinished);
}

DialogRunner::~DialogRunner()
{
    mLogger.Debug() << "Destroyed: " << this << "\n";
}

std::optional<BAK::TeleportIndex> 
DialogRunner::GetAndResetPendingTeleport()
{
    auto tmp = mPendingZoneTeleport;
    mPendingZoneTeleport.reset();
    return tmp;
}

void DialogRunner::SetDialogScene(IDialogScene* dialogScene)
{
    mDialogScene = dialogScene;
}

void DialogRunner::SetInWorldView(bool value)
{
    mInWorldView = value;
}

bool DialogRunner::IsActive()
{
    return mDialogState.mDialogActive 
        || mDialogState.mTooltipActive;
}

void DialogRunner::BeginDialog(
    BAK::Target target,
    bool isTooltip)
{
    mLastChoice.reset();

    ASSERT(!IsActive());

    mGameState.SetCharacterTextVariables();

    if (isTooltip)
        mDialogState.ActivateTooltip();
    else
        mDialogState.ActivateDialog();

    mTargetStack.push(target);
    mLogger.Debug() << "BeginDialog" << target << "\n";
    ContinueDialogFromStack();
}

bool DialogRunner::OnMouseEvent(const MouseEvent& event)
{
    return std::visit(overloaded{
        [this](const LeftMousePress& p){ return LeftMousePressed(p.mValue); },
        [this](const MouseMove& p){ return MouseMoved(p.mValue); },
        [](const auto& p){ return false; }
        },
        event);
}

bool DialogRunner::LeftMousePressed(glm::vec2)
{
    if (mDialogState.mTooltipActive)
    {
        mDialogState.ForceDeactivateTooltip(
        [this](){
            mDialogScene = nullptr;
            std::invoke(mFinished, GetLastChoice());
        });
    }
    else
    {
        RunDialog();
    }

    return true;
}

bool DialogRunner::MouseMoved(glm::vec2 pos)
{
    mDialogState.MouseMoved(pos);
    mDialogState.DeactivateTooltip(
        [this](){
            mDialogScene = nullptr;
            std::invoke(mFinished, GetLastChoice());
        });
    return false;
}

void DialogRunner::EvaluateSnippetActions()
{
    mLogger.Debug() << "Evaluating actions for " << mCurrentTarget << "\n";
    for (const auto& action : GetSnippet().mActions)
    {
        std::visit(overloaded{
            [&](const BAK::PushNextDialog& push){
                mLogger.Debug() << "Pushing: " << push.mTarget << "\n";
                mTargetStack.push(push.mTarget);
            },
            [&](const BAK::Teleport& teleport){
                mLogger.Debug() << "Teleporting to zoneIndex: " << teleport.mIndex << "\n";
                mPendingZoneTeleport = teleport.mIndex;
            },
            [&](const BAK::PlaySound& sound)
            {
                try
                {
                    mLogger.Debug() << "Playing sound: " << sound << "\n";
                    if (sound.mSoundIndex == 0)
                    {
                        // FIXME: Is this what 0 means?
                        //AudioA::AudioManager::Get().StopMusicTrack();
                    }
                    else if (sound.mSoundIndex < AudioA::MAX_SOUND)
                    {
                        AudioA::AudioManager::Get().PlaySound(AudioA::SoundIndex{sound.mSoundIndex});
                    }
                    else if (sound.mSoundIndex >= AudioA::MIN_SONG)
                    {
                        AudioA::AudioManager::Get().ChangeMusicTrack(AudioA::MusicIndex{sound.mSoundIndex});
                        // FIXME: Do we need a stack here?
                        mStartedMusic = true;
                    }
                }
                catch (std::exception& e)
                {
                    mLogger.Error() << " Playing sound: " << sound
                        << " failed with: " << e.what() << "\n";
                }
            },
            [&](const auto& a){
                mGameState.EvaluateAction(BAK::DialogAction{action});
            }},
            action);
    }
}

void DialogRunner::DisplaySnippet()
{
    std::string text{};
    if (!mRemainingText.empty())
        text = mRemainingText;
    else
    {
        text = GetSnippet().GetText();
        // Bit hacky: Add an extra line to ensure enough room to
        // display the query choice
        if (GetSnippet().IsQueryChoice())
            text += "\n";
    }
    auto nullDialog = NullDialogScene{};

    const auto [textDims, rem] = mDialogDisplay.DisplaySnippet(
        mDialogScene ? *mDialogScene : nullDialog,
        GetSnippet(),
        text,
        mInWorldView);
    mTextDims = textDims;
    mRemainingText = rem;
}

const BAK::DialogSnippet& DialogRunner::GetSnippet() const
{
    ASSERT(mCurrentTarget);
    return BAK::DialogStore::Get().GetSnippet(*mCurrentTarget);
}

std::optional<BAK::Target> DialogRunner::GetNextTarget()
{
    if (!mCurrentTarget)
    { 
        if (!mTargetStack.empty())
            return GetAndPopTargetStack();
        else
            return std::optional<BAK::Target>{};
    }

    const auto& snip = GetSnippet();
    if (snip.IsRandomChoice())
    {
        ASSERT(snip.GetChoices().size() > 0);
        const auto choice = GetRandomNumber(0, snip.GetChoices().size() - 1);
        return snip.GetChoices()[choice].mTarget;
    }
    else if (snip.GetChoices().size() >= 1)
    {
        for (const auto& c : snip.GetChoices())
        {
            if (mGameState.EvaluateDialogChoice(c.mChoice))
                return c.mTarget;
        }
    }

    if (!mTargetStack.empty())
    {
        return GetAndPopTargetStack();
    }

    return std::optional<BAK::Target>{};
}

void DialogRunner::MakeChoice(BAK::ChoiceIndex choice)
{
    mLogger.Debug() << "Made choice: " << choice << "\n";
    mScreenStack.PopScreen();

    const auto& choices = GetSnippet().GetChoices();
    const auto it = std::find_if(choices.begin(), choices.end(),
        [choice](const auto& c){
            if (std::holds_alternative<BAK::ConversationChoice>(c.mChoice))
                return std::get<BAK::ConversationChoice>(c.mChoice).mEventPointer == choice.mValue;
            else if (std::holds_alternative<BAK::QueryChoice>(c.mChoice))
                return std::get<BAK::QueryChoice>(c.mChoice).mQueryIndex == choice.mValue;
            else
                return false;
        });

    mLastChoice = choice;

    if (it == choices.end())
    {
        // usual "Yes/No" dialogs
        if (choices.size() == 0 || choices.size() == 1)
        {
            mLogger.Info() << "Yes|No dialog choice. Player chose: " << mLastChoice << "\n";
        }
        else
        {
            // Break out of the question loop
            ASSERT(!mTargetStack.empty());
            mTargetStack.pop();
        }
    }
    else
    {
        evaluate_if<BAK::ConversationChoice>(
            it->mChoice,
            [&](const auto& c){
                mGameState.MarkDiscussed(c);
            });

        mTargetStack.push(it->mTarget);
    }

    ContinueDialogFromStack();
}

void DialogRunner::ShowDialogChoices()
{
    mLogger.Debug() << "DialogChoices: " << GetSnippet() << "\n";
    mChoices.SetPosition(glm::vec2{15, 125});
    mChoices.SetDimensions(glm::vec2{295, 66});

    auto choices = std::vector<std::pair<BAK::ChoiceIndex, std::string>>{};
    unsigned i = 0;
    for (const auto& c : GetSnippet().GetChoices())
    {
        if (std::holds_alternative<BAK::ConversationChoice>(c.mChoice))
        {
            const auto choice = std::get<BAK::ConversationChoice>(c.mChoice);
            if (mGameState.GetEventStateBool(choice.mEventPointer)
                && !mGameState.CheckInhibited(choice))
            {
                const auto fontStyle = mGameState.CheckDiscussed(choice)
                    ? '\xf4' // unbold
                    : '#';
                choices.emplace_back(
                    std::make_pair(
                        BAK::ChoiceIndex{choice.mEventPointer},
                        fontStyle + std::string{
                            mKeywords.GetDialogChoice(choice.mEventPointer)}));

            }
        }
    }
    choices.emplace_back(std::make_pair(-1, "Goodbye"));
    constexpr auto buttonSize = glm::vec2{68, 14};
    mChoices.StartChoices(choices, buttonSize);

    mDialogDisplay.Clear();
    if (mDialogScene)
    {
        mDialogDisplay.ShowWorldViewPane(mInWorldView);
        mDialogDisplay.DisplayPlayer(*mDialogScene, GetSnippet().mActor);
    }
    mScreenStack.PushScreen(&mChoices);
}

void DialogRunner::ShowQueryChoices()
{
    mLogger.Debug() << "QueryChoices: " << GetSnippet() << "\n";
    mLogger.Debug() << "Last text dims:" << mTextDims << "\n";
    mChoices.SetPosition(glm::vec2{100, mTextDims.y});
    mChoices.SetDimensions(glm::vec2{285, 66});

    auto choices = std::vector<std::pair<BAK::ChoiceIndex, std::string>>{};
    unsigned i = 0;
    for (const auto& c : GetSnippet().GetChoices())
    {
        if (std::holds_alternative<BAK::QueryChoice>(c.mChoice))
        {
            const auto index = std::get<BAK::QueryChoice>(c.mChoice).mQueryIndex;
            mLogger.Debug() << " Choice Index: " << index << "\n";
            choices.emplace_back(
                std::make_pair(
                    BAK::ChoiceIndex{index},
                    "#" + std::string{
                        mKeywords.GetQueryChoice(index)}));
        }
        else
        {
            throw std::runtime_error("Non-query choice in query choice display");
        }
    }

    if (choices.size() == 0)
    {
        const auto index = BAK::Keywords::sYesIndex; // Yes
        choices.emplace_back(
            std::make_pair(
                BAK::ChoiceIndex{index},
                "#" + std::string{mKeywords.GetQueryChoice(index)}));
    }

    if (choices.size() == 1)
    {
        const auto availableChoice = choices.back().first;
        const auto index = availableChoice.mValue == BAK::Keywords::sNoIndex ? BAK::Keywords::sYesIndex : BAK::Keywords::sNoIndex;
        choices.emplace_back(
            std::make_pair(
                BAK::ChoiceIndex{index},
                "#" + std::string{mKeywords.GetQueryChoice(index)}));
    }
    

    constexpr auto buttonSize = glm::vec2{32, 14};
    mChoices.StartChoices(choices, buttonSize);

    mScreenStack.PushScreen(&mChoices);
}

void DialogRunner::ContinueDialogFromStack()
{
    mRemainingText.clear();
    mCurrentTarget.reset();
    RunDialog();
}

bool DialogRunner::RunDialog()
{
    mLogger.Debug() << "RunDialog rem: ["
        << mRemainingText << "] CurTarg: " << mCurrentTarget <<"\n";

    if (!mRemainingText.empty())
    {
        DisplaySnippet();
        if (mRemainingText.empty() 
            && GetSnippet().IsQueryChoice())
            ShowQueryChoices();

        return true;
    }

    if (mCurrentTarget && GetSnippet().IsQueryChoice())
    {
        ShowQueryChoices();
        return true;
    }

    do
    {
        const auto nextTarget = GetNextTarget();
        mLogger.Debug() << "Curr: " << mCurrentTarget 
            << " Next Target: " << nextTarget << "\n";
        if (!nextTarget 
            || (std::holds_alternative<BAK::KeyTarget>(*nextTarget) 
            && std::get<BAK::KeyTarget>(*nextTarget) == BAK::KeyTarget{0x0}))
        {
            CompleteDialog();
            return false;
        }
        else
        {
            mCurrentTarget = *nextTarget;
            EvaluateSnippetActions();
            mLogger.Debug() << "Progressing through: " << GetSnippet() << "\n";
        }
    } while (!mCurrentTarget
        || !GetSnippet().IsDisplayable());

    if (GetSnippet().IsDialogChoice())
        ShowDialogChoices();
    else
        DisplaySnippet();

    if (GetSnippet().IsQueryChoice() && mRemainingText.empty())
        ShowQueryChoices();

    return true;
}

void DialogRunner::CompleteDialog()
{
    mLogger.Debug() << "Finished dialog\n";
    mDialogDisplay.Clear();
    mDialogState.DeactivateDialog();
    mDialogScene = nullptr;
    mCurrentTarget.reset();
    mRemainingText.clear();
    if (mStartedMusic)
    {
        AudioA::AudioManager::Get().PopTrack();
        mStartedMusic = false;
    }
    std::invoke(mFinished, GetLastChoice());
    // reset last choice...?
}

BAK::Target DialogRunner::GetAndPopTargetStack()
{
    ASSERT(!mTargetStack.empty());
    const auto target = mTargetStack.top();
    mTargetStack.pop();
    return target;
}

}
