#include "bak/gameState.hpp"

#include "bak/camp.hpp"

namespace BAK {

void GameState::ElapseTime(Time time)
{
    // need to accumulate these and commit when the
    // dialog is over..?
    auto splitTime = time;
    auto camp = BAK::MakeCamp{*this};
    bool dialogResetsSleep = time > Times::TwelveHours;
    while (splitTime > Times::OneHour)
    {
        if (dialogResetsSleep)
        {
            GetWorldTime().SetTimeLastSlept(
                GetWorldTime().GetTime());
        }
        splitTime = splitTime - Times::OneHour;
        camp.HandleGameTimeChange(
            Times::OneHour,
            true,
            dialogResetsSleep,
            false,
            0);
    }

    if (splitTime > Time{0})
    {
        camp.HandleGameTimeChange(
            splitTime,
            true,
            dialogResetsSleep,
            false,
            0);
    }
}

}
