#pragma once

#include "bak/types.hpp"

#include <cstdint>
#include <vector>
#include <string>

namespace BAK {

class MonsterNames
{
    static constexpr std::string sInvalidMonster = "INVALID MONSTER";
public:
    static const MonsterNames& Get()
    {
        static auto mnames = MonsterNames{};
        return mnames;
    }

    struct Monster
    {
        Monster(
            std::string prefix,
            std::uint8_t unknown0,
            std::uint8_t unknown1,
            std::uint8_t unknown2,
            std::uint8_t colorSwap)
        :
            mPrefix{prefix},
            mUnknown0{unknown0},
            mUnknown1{unknown1},
            mUnknown2{unknown2},
            mColorSwap{colorSwap}
        {}

        std::string mPrefix;
        std::uint8_t mUnknown0;
        std::uint8_t mUnknown1;
        std::uint8_t mUnknown2;
        std::uint8_t mColorSwap;
    };

    const std::string& GetMonsterName(MonsterIndex monster) const
    {
        if (monster.mValue < mMonsterNames.size())
        {
            return mMonsterNames[monster.mValue];
        }
        else
        {
            return sInvalidMonster;
        }
    }

    const std::string& GetMonsterAnimationFile(MonsterIndex monster) const
    {
        ASSERT(monster.mValue < mMonsterPrefixes.size());
        return mMonsterPrefixes[monster.mValue].mPrefix;
    }

    auto GetColorSwap(MonsterIndex monster) const
    {
        ASSERT(monster.mValue < mMonsterPrefixes.size());
        return mMonsterPrefixes[monster.mValue].mColorSwap;
    }

    auto size() const { return mMonsterPrefixes.size(); }

private:
    MonsterNames();

    std::vector<std::string> mMonsterNames;
    std::vector<Monster> mMonsterPrefixes;
};

}
