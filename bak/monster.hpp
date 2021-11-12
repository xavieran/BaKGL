#pragma once

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

private:
    std::vector<std::string> mMonsterNames;
    std::vector<Monster> mMonsterPrefixes;
};

}
