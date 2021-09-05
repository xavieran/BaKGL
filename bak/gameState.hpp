#pragma once

#include "bak/dialog.hpp"
#include "bak/types.hpp"

namespace BAK {

class GameState
{
public:
    GameState()
    :
        mPartyLeader{
            "Locklear",
            1},
        mPartyFollower{
            "Owyn",
            2}
    {}

    struct Character
    {
        std::string mName;
        unsigned mIndex;
    };

    Chapter GetChapter() const
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

    auto GetShopType() const
    {
        return 4;
    }

    const Character& GetPartyLeader()
    {
        return mPartyLeader;
    }

    // Return random person who's not the leader...
    const Character& GetPartyFollower()
    {
        return mPartyFollower;
    }

    Character mPartyLeader;
    Character mPartyFollower;
};

}
