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
    static const MonsterNames& Get();

    struct Monster
    {
        Monster(
            std::string prefix,
            std::uint8_t unknown0,
            std::uint8_t unknown1,
            std::uint8_t unknown2,
            std::uint8_t colorSwap);

        std::string mPrefix;
        std::uint8_t mSuffix0;
        std::uint8_t mSuffix1;
        std::uint8_t mSuffix2;
        std::uint8_t mColorSwap;
    };

    const std::string& GetMonsterName(MonsterIndex monster) const;
    const std::string& GetMonsterAnimationFile(MonsterIndex monster) const;
    std::uint8_t GetColorSwap(MonsterIndex monster) const;
    Monster GetMonster(MonsterIndex monster) const
    {
        return mMonsterPrefixes[monster.mValue - 1];
    }
    std::size_t size() const;

private:
    MonsterNames();

    std::vector<std::string> mMonsterNames;
    std::vector<Monster> mMonsterPrefixes;
};

std::ostream& operator<<(std::ostream& os, const MonsterNames::Monster& m);
}
