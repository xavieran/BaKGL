#pragma once

#include "bak/dialog.hpp"

namespace BAK {

class GameState
{
public:
    static constexpr std::string_view mPartyLeader = "Locklear";
    static constexpr std::string_view mPartyFollower = "Owyn";

    auto GetChapter() const
    {
        return 1;
    }

    auto GetMoney() const
    {
        return 1000;
    }

    auto GetTime() const
    {
        return 0; // daytime
        //return 1; // nighttime
    }

    std::string_view GetPartyLeader()
    {
        return mPartyLeader;
    }

    // Return random person who's not the leader...
    std::string_view GetPartyFollower()
    {
        return mPartyFollower;
    }
};

}
