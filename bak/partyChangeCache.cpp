#include "bak/partyChangeCache.hpp"

#include "bak/dialogSources.hpp"
#include "bak/gameState.hpp"
#include "bak/party.hpp"
#include "bak/skills.hpp"

#include <optional>

namespace BAK {

void PartyChangeCache::CacheState(GameState& gs)
{
    for (unsigned i = 0; i < sMaxCharacters; i++)
    {
        const auto& character = gs.GetParty().GetCharacter(CharIndex{i});
        mCachedConditions[i] = character.GetConditions();
        std::uint16_t bitmask = 0;
        for (unsigned s = 0; s < Skills::sSkills; s++)
        {
            if (character.GetSkills().GetSkill(
                static_cast<SkillType>(s)).mUnseenImprovement)
                bitmask |= (1 << s);
        }
        mCachedUnseenImprovements[i] = bitmask;
    }
}

void PartyChangeCache::ClearCondition(const GameState& gs, Condition condition)
{
    for (unsigned i = 0; i < sMaxCharacters; i++)
    {
        const auto& character = gs.GetParty().GetCharacter(CharIndex{i});
        mCachedConditions[i].SetCondition(
            condition,
            character.GetConditions().GetCondition(condition).Get());
    }
}

std::optional<PartyChangeResult> PartyChangeCache::CheckForDeath(GameState& gameState)
{
    bool anyAlive = false;
    gameState.GetParty().ForEachActiveCharacter([&](const auto& character) {
        auto nearDeath = character.GetConditions()
            .GetCondition(BAK::Condition::NearDeath).Get();
        if (nearDeath == 0)
        {
            anyAlive = true;
        }
        return Loop::Continue;
    });

    if (!anyAlive)
    {
        return PartyChangeResult{
            true, DialogSources::mDeathDueToCondition, true};
    }

    return std::nullopt;
}

std::optional<PartyChangeResult> PartyChangeCache::CheckSkillImprovements(GameState& gameState)
{
    std::optional<SkillType> whichSkill{};
    std::optional<CharIndex> who{};
    unsigned howManySkills{};
    unsigned howManyPeople{};

    gameState.GetParty().ForEachActiveCharacter([&](const auto& character) {
        const unsigned charIdx = character.GetIndex().mValue;
        bool skillImproved = false;
        for (unsigned s = 0; s < Skills::sSkills; s++)
        {
            auto skill = static_cast<SkillType>(s);
            const auto current = character.GetSkills()
                .GetSkill(skill).mUnseenImprovement;
            const auto cached = (mCachedUnseenImprovements[charIdx] >> s) & 1;
            if (current && !cached)
            {
                mCachedUnseenImprovements[charIdx] |= (1 << s);
                if (!whichSkill)
                {
                    howManySkills = 1;
                }
                else if (skill != *whichSkill)
                {
                    howManySkills++;
                }
                skillImproved = true;
                whichSkill = skill;
            }
        }

        if (skillImproved)
        {
            who = character.GetIndex();
            howManyPeople++;
        }

        return Loop::Continue;
    });

    if (whichSkill)
    {
        assert(who);
        gameState.SetActiveCharacter(*who);
        gameState.SetImprovedSkill(*whichSkill);
        Target dialog{};
        if (howManySkills > 1 && howManyPeople > 1)
        {
            dialog = DialogSources::mSkillImprovedManySkillsManyMembers;
        }
        if (howManySkills > 1)
        {
            dialog = DialogSources::mSkillImprovedManySkillsOneMember;
        }
        else if (howManyPeople > 1)
        {
            dialog = DialogSources::mSkillImprovedOneSkillManyMembers;
        }
        else
        {
            dialog = DialogSources::mSkillImprovedOneSkillOneMember;
        }
        return PartyChangeResult{true, dialog};
    }

    return std::nullopt;
}

std::optional<PartyChangeResult> PartyChangeCache::CheckNewConditions(GameState& gameState, bool inInn)
{
    for (unsigned condIdx = 0; condIdx < Conditions::sNumConditions; condIdx++)
    {
        const auto cond = static_cast<Condition>(condIdx);
        if (inInn && cond == Condition::NearDeath) continue;
        const auto notification = DialogSources::GetConditionNotification(cond);
        if (!notification) continue;

        unsigned howManyAffected = 0;

        gameState.GetParty().ForEachActiveCharacter([&](const auto& character) {
            const unsigned charIdx = character.GetIndex().mValue;
            const auto oldVal = mCachedConditions[charIdx].GetCondition(cond).Get();
            const auto newVal = character.GetConditions().GetCondition(cond).Get();
            if (oldVal == 0 && newVal != 0)
            {
                mCachedConditions[charIdx].SetCondition(cond, newVal);
                howManyAffected++;
                gameState.SetActiveCharacter(character.GetIndex());
            }
            return Loop::Continue;
        });

        if (howManyAffected > 0)
        {
            gameState.SetDialogContext_7530(howManyAffected > 1 ? 0 : 1);
            return PartyChangeResult{true, *notification};
        }
    }

    return std::nullopt;
}

PartyChangeResult PartyChangeCache::CheckPartyChanges(GameState& gameState, bool camping, bool inInn)
{
    if (!inInn)
    {
        if (auto result = CheckForDeath(gameState))
        {
            return *result;
        }
    }

    if (!camping)
    {
        if (auto result = CheckSkillImprovements(gameState))
        {
            return *result;
        }
    }

    if (auto result = CheckNewConditions(gameState, inInn))
    {
        return *result;
    }

    return PartyChangeResult{};
}

}

