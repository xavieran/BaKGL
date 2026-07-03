#pragma once

#include "bak/types.hpp"

#include <cstdint>
#include <vector>
#include <string>

namespace BAK {


static constexpr auto sMoredhelWarrior      = MonsterIndex{18};
static constexpr auto sBrakNurr             = MonsterIndex{19};
static constexpr auto sMoredhelSpellcaster  = MonsterIndex{21};
static constexpr auto sBlackslayer          = MonsterIndex{22};
static constexpr auto sNighthawk            = MonsterIndex{23};
static constexpr auto sPantathian           = MonsterIndex{25};
static constexpr auto sPanthTiandn          = MonsterIndex{26};
static constexpr auto sServitorOfLimsKragma = MonsterIndex{28};
static constexpr auto sRogueMage            = MonsterIndex{30};
static constexpr auto sBulldrakeWyvern      = MonsterIndex{41};
static constexpr auto sGrandsireWyvern      = MonsterIndex{42};
static constexpr auto sHatchlingWyvern      = MonsterIndex{43};
static constexpr auto sTroll                = MonsterIndex{48};
static constexpr auto sDread                = MonsterIndex{49};
static constexpr auto sWindElemental        = MonsterIndex{54};
static constexpr auto sNethermander         = MonsterIndex{58};
static constexpr auto sRusalka              = MonsterIndex{56};
static constexpr auto sShade                = MonsterIndex{57};

class MonsterNames
{
    static constexpr std::string_view sInvalidMonster = "INVALID MONSTER";
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

    std::string_view GetMonsterName(MonsterIndex monster) const;
    const std::string& GetMonsterAnimationFile(MonsterIndex monster) const;
    std::uint8_t GetColorSwap(MonsterIndex monster) const;
    Monster GetMonster(MonsterIndex monster) const
    {
        return mMonsterPrefixes[monster.mValue];
    }
    std::size_t size() const;

private:
    MonsterNames();

    std::vector<std::string> mMonsterNames;
    std::vector<Monster> mMonsterPrefixes;
};

std::ostream& operator<<(std::ostream& os, const MonsterNames::Monster& m);

bool IsMonsterGhost(MonsterIndex);
}
