#include "bak/chapterTransitions.hpp"

#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/startupFiles.hpp"

#include "bak/state/event.hpp"

namespace BAK {

void TransitionToChapter(Chapter chapter, GameState& gs)
{
    auto currentGold = gs.GetParty().GetGold();
    auto time = gs.GetWorldTime().GetTime();
    time += Times::OneDay;
    auto remainder = Time{time.mTime % Times::OneDay.mTime};
    time -= remainder;
    gs.GetWorldTime().SetTime(time);
    
    if (chapter != Chapter{1})
    {
        // combatCompleted = 1;
        auto offset = 0x12f7 + ((chapter.mValue - 1) << 2);
        // writeDataToFile(partyGold, ds, offset, 4);
    }

    // loc_41b71
    for (unsigned i = 0; i < 80; i++)
    {
        gs.ReduceAndEvaluateTimeExpiringState(Time{0x7530});
    }
    const auto startLocation = LoadChapterStartLocation(chapter);
    gs.SetLocation(startLocation.mLocation);
    // gs.SetMapLocation(startLocation.mMapLocation);

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

    //gs.ClearActiveSpells();
    
    // switch (chapter)...

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

    // RunDialogActions DialogSources::mEndOfChapterResetFlags;
}

}
