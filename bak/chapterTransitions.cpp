#include "bak/chapterTransitions.hpp"

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/startupFiles.hpp"

#include "bak/state/event.hpp"
#include "bak/state/money.hpp"

namespace BAK {

std::optional<BAK::Teleport> TransitionToChapter(Chapter chapter, GameState& gs)
{
    gs.SetChapter(chapter);

    auto currentGold = gs.GetParty().GetGold();
    auto time = gs.GetWorldTime().GetTime();
    time += Times::OneDay;
    auto remainder = Time{time.mTime % Times::OneDay.mTime};
    time -= remainder;
    gs.GetWorldTime().SetTime(time);

    if (chapter == Chapter{1})
    {
        gs.GetWorldTime().SetTime(Time{0x189c0});
    }
    
    if (chapter != Chapter{1})
    {
        // combatCompleted = 1;
        gs.Apply(State::WritePartyMoney, chapter, gs.GetParty().GetGold());
    }

    // loc_41b71
    for (unsigned i = 0; i < 80; i++)
    {
        gs.ReduceAndEvaluateTimeExpiringState(Time{0x7530});
    }
    const auto startLocation = LoadChapterStartLocation(chapter);
    gs.SetLocation(startLocation.mLocation);
    gs.SetMapLocation(startLocation.mMapLocation);

    // mov     eax, [bp+currentGold]
    // add     partyGold, eax
    // mov     eax, [bp+currentTime]
    // add     currentTime, eax
    // mov     eax, currentTime
    // mov     timeOfLastSleep?, eax
    // mov     zoneNUmber, 0
    
    // Set all encounter unique state flags back to false
    for (unsigned i = 0; i < 0x12c0; i++)
    {
        gs.Apply(State::SetEventFlagFalse, 0x190 + i);
    }

    switch (chapter.mValue)
    {
        case 1:
           break;
        case 2:
        {
            auto* lockysRoom = gs.GetContainerForGDSScene(HotspotRef{2, 'B'});
            auto& locky = gs.GetParty().GetCharacter(Locklear);
            lockysRoom->GetInventory().CopyFrom(locky.GetInventory());
            locky.GetInventory().GetItems().clear();
        } break;
        case 3:
            gs.SetEventValue(0x1fbc, 0);
            break;
        case 4:
            //gs.SetMoney(0);
            break;
        case 5:[[fallthrough]];
        case 6:[[fallthrough]];
        case 7:[[fallthrough]];
            //const auto goldFromPriorChapter = gs.Apply(State::ReadPartyMoney, Chapter{chapter.mValue - 1});
            //gs.SetMoney(goldFromPriorChapter);
        case 8:[[fallthrough]];
        case 9:[[fallthrough]];
        default:
           break;
    }

    gs.GetWorldTime().SetTimeLastSlept(gs.GetWorldTime().GetTime());
    gs.GetParty().ForEachActiveCharacter([](auto& character){
        auto& skills = character.GetSkills();
        auto& conditions = character.GetConditions();
        for (unsigned i = 0; i < Conditions::sNumConditions; i++)
        {
            conditions.AdjustCondition(skills, static_cast<Condition>(i), -100);
        }
        DoAdjustHealth(skills, conditions, 100, 0x7fff);
        for (unsigned i = 0; i < 16; i++)
        {
            skills.GetSkill(static_cast<SkillType>(i)).mUnseenImprovement = false;
        }
        return Loop::Continue;
    });

    // Evaluate the start of chapter actions
    const auto& ds = DialogStore::Get();
    auto startOfChapter= ds.GetSnippet(DialogSources::mStartOfChapterActions);
    assert(startOfChapter.GetChoices().size() == 1);
    auto startOfChapterFlagsReset = ds.GetSnippet(startOfChapter.GetChoices()[0].mTarget);
    for (const auto& action : startOfChapterFlagsReset.GetActions())
    {
        gs.EvaluateAction(action);
    }

    assert(startOfChapter.GetActions().size() == 1);
    auto* chapterActions = &ds.GetSnippet(
        ds.GetSnippet(std::get<PushNextDialog>(startOfChapter.mActions[0]).mTarget).GetChoices()[chapter.mValue - 1].mTarget);

    std::optional<BAK::Teleport> teleport{};
    auto CheckTeleport = [&](const auto& action)
    {
        if (std::holds_alternative<BAK::Teleport>(action))
        {
            teleport = std::get<BAK::Teleport>(action);
        }
    };

    do {
        for (const auto& action : chapterActions->GetActions())
        {
            gs.EvaluateAction(action);
            CheckTeleport(action);
        }
        for (const auto& choice : chapterActions->GetChoices())
        {
            if (gs.EvaluateDialogChoice(choice))
            {
                chapterActions = &ds.GetSnippet(choice.mTarget);
            }
        }
    } while (!chapterActions->GetChoices().empty());

    for (const auto& action : chapterActions->GetActions())
    {
        gs.EvaluateAction(action);
        CheckTeleport(action);
    }

    return teleport;
}

}
