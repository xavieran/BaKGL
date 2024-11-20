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
        std::uint8_t mUnknown0;
        std::uint8_t mUnknown1;
        std::uint8_t mUnknown2;
        std::uint8_t mColorSwap;
    };

    const std::string& GetMonsterName(MonsterIndex monster) const;
    const std::string& GetMonsterAnimationFile(MonsterIndex monster) const;
    std::uint8_t GetColorSwap(MonsterIndex monster) const;
    std::size_t size() const;

private:
    MonsterNames();

    std::vector<std::string> mMonsterNames;
    std::vector<Monster> mMonsterPrefixes;
};

}
