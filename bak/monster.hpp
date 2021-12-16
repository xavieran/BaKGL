#pragma once

#include "bak/types.hpp"

#include <cstdint>
#include <vector>
#include <string>

namespace BAK {

class MonsterNames
{
public:
    MonsterNames();

    struct Monster
    {
        Monster(
            std::string prefix,
            std::uint8_t unknown0,
            std::uint8_t unknown1,
            std::uint8_t unknown2,
            std::uint8_t unknown3)
        :
            mPrefix{prefix},
            mUnknown0{unknown0},
            mUnknown1{unknown1},
            mUnknown2{unknown2},
            mUnknown3{unknown3}
        {}

        std::string mPrefix;
        std::uint8_t mUnknown0;
        std::uint8_t mUnknown1;
        std::uint8_t mUnknown2;
        std::uint8_t mUnknown3;
    };

    const std::string& GetMonsterAnimationFile(MonsterIndex monster) const
    {
        ASSERT(monster.mValue < mMonsterPrefixes.size());
        return mMonsterPrefixes[monster.mValue].mPrefix;
    }

    auto GetUnknown3(MonsterIndex monster) const
    {
        ASSERT(monster.mValue < mMonsterPrefixes.size());
        return mMonsterPrefixes[monster.mValue].mUnknown3;
    }

    auto size() const { return mMonsterPrefixes.size(); }

private:
    std::vector<std::string> mMonsterNames;
    std::vector<Monster> mMonsterPrefixes;
};

}
