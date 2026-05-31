#include "bak/state/condition.hpp"

#include "bak/condition.hpp"

#include "bak/state/event.hpp"
#include "bak/state/offsets.hpp"

#include "bak/types.hpp"

#include "bak/file/fileBuffer.hpp"

namespace BAK::State {

void SyncConditionEventFlags(
    FileBuffer& fb,
    unsigned charIndex,
    bool inCombat,
    const Conditions& conditions)
{
    for (unsigned i = 0; i < Conditions::sNumConditions; i++)
    {
        const auto cond = static_cast<Condition>(i);

        if (cond == Condition::Healing || cond == Condition::Drunk)
            continue;

        if (cond == Condition::NearDeath && inCombat)
            continue;

        const auto isActive = conditions.GetCondition(cond).Get() != 0;
        const auto eventPtr = sConditionStateEventFlag
            + charIndex * Conditions::sNumConditions
            + i;

        SetEventFlag(fb, eventPtr, isActive ? 1 : 0);
    }
}

bool IsConditionEvent(unsigned eventPtr)
{
    constexpr auto sConditionStateEnd = sConditionStateEventFlag
        + sMaxCharacters * Conditions::sNumConditions;
    return eventPtr >= sConditionStateEventFlag && eventPtr < sConditionStateEnd;
}

}
