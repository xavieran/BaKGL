#include "bak/chapterTransitions.hpp"

#include "bak/constants.hpp"
#include "bak/coordinates.hpp"
#include "bak/dialogAction.hpp"
#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/hotspotRef.hpp"
#include "bak/itemNumbers.hpp"
#include "bak/itemInteractions.hpp"
#include "bak/objectInfo.hpp"
#include "bak/startupFiles.hpp"

#include "bak/state/event.hpp"
#include "bak/state/money.hpp"

namespace BAK {

std::optional<BAK::Teleport> TransitionToChapter(Chapter chapter, GameState& gs)
{
    Logging::LogInfo(__FUNCTION__) << "Chapter: " << chapter << "\n";
    gs.SetChapter(chapter);

    const auto startLocation = LoadChapterStartLocation(chapter);
    gs.SetLocation(startLocation.mLocation);
    gs.SetMapLocation(startLocation.mMapLocation);

    auto time = gs.GetWorldTime().GetTime() + Times::OneDay;
    time -= Time{(time.mTime % Times::OneDay.mTime)};
    time += startLocation.mTimeElapsed;
    gs.GetWorldTime().SetTime(time);
    gs.GetWorldTime().SetTimeLastSlept(gs.GetWorldTime().GetTime());

    if (chapter != Chapter{1})
    {
        // combatCompleted = 1;
        gs.Apply(State::WritePartyMoney, chapter, gs.GetParty().GetGold());
    }

    // is this even right....?
    for (unsigned i = 0; i < 80; i++)
    {
        gs.ReduceAndEvaluateTimeExpiringState(Time{0x7530});
    }

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
            assert(lockysRoom);
            auto& locky = gs.GetParty().GetCharacter(Locklear);
            lockysRoom->GetInventory().GetItems().clear();
            lockysRoom->GetInventory().CopyFrom(locky.GetInventory());
        } break;
        case 3:
        {
            gs.SetEventValue(0x1fbc, 0);
        } break;
        case 4:
        {
            auto* owynChest = gs.GetWorldContainer(ZoneNumber{12}, GamePosition{694800, 700800});
            assert(owynChest);
            auto& owyn = gs.GetParty().GetCharacter(Owyn);
            owynChest->GetInventory().CopyFrom(owyn.GetInventory());
            owyn.GetInventory().GetItems().clear();

            auto torch = InventoryItemFactory::MakeItem(sTorch, 6);
            owyn.GiveItem(torch);

            auto* gorathChest = gs.GetWorldContainer(ZoneNumber{12}, GamePosition{698400, 696800});
            assert(gorathChest);
            auto& gorath = gs.GetParty().GetCharacter(Gorath);
            gorathChest->GetInventory().CopyFrom(gorath.GetInventory());
            gorath.GetInventory().GetItems().clear();

            gorath.GiveItem(torch);
            UseItem(gs, gorath, InventoryIndex{0});

            gs.GetParty().SetMoney(Royals{0});
        } break;
        case 5:
        {
            auto* locklearCh5Inventory = gs.GetWorldContainer(ZoneNumber{0}, {10, 0});
            assert(locklearCh5Inventory);
            auto& locky = gs.GetParty().GetCharacter(Locklear);
            locky.GetInventory().GetItems().clear();
            locky.GetInventory().CopyFrom(locklearCh5Inventory->GetInventory());
            {
                auto it = locky.GetInventory().FindItemType(BAK::ItemType::Sword);
                assert(it != locky.GetInventory().GetItems().end());
                it->SetEquipped(true);
            }

            {
                auto it = locky.GetInventory().FindItemType(BAK::ItemType::Armor);
                assert(it != locky.GetInventory().GetItems().end());
                it->SetEquipped(true);
            }

            {
                auto it = locky.GetInventory().FindItemType(BAK::ItemType::Crossbow);
                assert(it != locky.GetInventory().GetItems().end());
                it->SetEquipped(true);
            }

            gs.GetParty().SetMoney(gs.Apply(State::ReadPartyMoney, Chapter{4}));
                
        } break;
        case 6:
        {
            // I can't quite comprehend what the point of this was...
            auto* refInn1 = gs.GetWorldContainer(ZoneNumber{0}, {20, 1});
            assert(refInn1);
            auto* inn1 = gs.GetContainerForGDSScene(HotspotRef{60, 'C'});
            assert(inn1);
            inn1->GetInventory().GetItems().clear();
            inn1->GetInventory().CopyFrom(refInn1->GetInventory());

            auto* refInn2 = gs.GetWorldContainer(ZoneNumber{0}, {30, 1});
            assert(refInn2);
            auto* inn2 = gs.GetContainerForGDSScene(HotspotRef{64, 'C'});
            assert(inn2);
            inn2->GetInventory().GetItems().clear();
            inn2->GetInventory().CopyFrom(refInn2->GetInventory());

            gs.GetParty().SetMoney(gs.Apply(State::ReadPartyMoney, Chapter{5}));
        } break;
        case 7:
            gs.GetParty().SetMoney(gs.Apply(State::ReadPartyMoney, Chapter{6}));
            gs.SetEventValue(0x1ab1, 1);
            break;
        case 8:
            gs.GetParty().SetMoney(gs.Apply(State::ReadPartyMoney, Chapter{7}));
        case 9:
        default:
           break;
    }

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
    auto startOfChapter = ds.GetSnippet(DialogSources::mStartOfChapterActions);
    Logging::LogInfo(__FUNCTION__) << "Evaluating snippet: " << startOfChapter << "\n";
    assert(startOfChapter.GetChoices().size() == 1);
    auto startOfChapterFlagsReset = ds.GetSnippet(startOfChapter.GetChoices()[0].mTarget);
    for (const auto& action : startOfChapterFlagsReset.GetActions())
    {
        gs.EvaluateAction(action);
    }

    assert(startOfChapter.GetActions().size() == 1);
    auto* chapterActions = &ds.GetSnippet(
        ds.GetSnippet(std::get<PushNextDialog>(startOfChapter.mActions[0]).mTarget).GetChoices()[chapter.mValue - 1].mTarget);

    auto* prevActions = chapterActions;

    std::optional<BAK::Teleport> teleport{};
    auto CheckTeleport = [&](const auto& action)
    {
        if (std::holds_alternative<BAK::Teleport>(action))
        {
            assert(!teleport);
            teleport = std::get<BAK::Teleport>(action);
        }
    };

    do {
        Logging::LogInfo(__FUNCTION__) << "Evaluating snippet: " << *chapterActions << "\n";
        prevActions = chapterActions;
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

    if (prevActions != chapterActions)
    {
        Logging::LogInfo(__FUNCTION__) << "Evaluating final actions for snippet: " << *chapterActions << "\n";
        for (const auto& action : chapterActions->GetActions())
        {
            gs.EvaluateAction(action);
            CheckTeleport(action);
        }
    }

    return teleport;
}

}
