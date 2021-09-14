#pragma once

#include "bak/dialog.hpp"
#include "bak/gameData.hpp"
#include "bak/types.hpp"

namespace BAK {

class GameState
{
public:
    GameState()
    :
        GameState{nullptr}
    {}

    GameState(
        GameData* gameData)
    :
        mPartyLeader{
            "Locklear",
            1},
        mPartyFollower{
            "Owyn",
            2},
        mGameData{gameData}
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

    bool GetEventState(unsigned eventPtr) const
    {
        if (mEventState.contains(eventPtr))
            return mEventState.at(eventPtr);
        else if (mGameData != nullptr)
            return mGameData->ReadEvent(eventPtr);
        else
            return false;
    }

    void SetEventState(unsigned eventPtr)
    {
        mEventState.emplace(eventPtr, true);
    }

    Character mPartyLeader;
    Character mPartyFollower;
    GameData* mGameData;
    std::unordered_map<unsigned, bool> mEventState;
};

}
