#include "bak/state/condition.hpp"

#include "bak/condition.hpp"
#include "bak/state/offsets.hpp"

#include "bak/types.hpp"

namespace BAK::State {

bool IsConditionEvent(unsigned eventPtr)
{
    constexpr auto sConditionStateEnd = sConditionStateEventFlag
        + sMaxCharacters * Conditions::sNumConditions;
    return eventPtr >= sConditionStateEventFlag && eventPtr < sConditionStateEnd;
}

}
