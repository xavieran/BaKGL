#pragma once

#include <string>

namespace BAK {

static constexpr float gWorldScale = 100.;

enum class Terrain 
{
    Ground    = 0,
    Road      = 1,
    Waterfall = 2,
    Path      = 3,
    Dirt      = 4,
    River     = 5,
    Sand      = 6,
    Bank      = 7
};

enum class Enemy 
{
    SolidCrystal = 0x09,
    TransparentCrystal = 0x0a,
    Blaster = 0x0b,
    SolidOctagon = 0x0c,
    LavaOctagon = 0x0d,
    Unknown = 0x0e,
    Gorath = 0x0f,
    Owyn = 0x10,
    Locklear = 0x11,
    Moredhel = 0x12,
    BrakNurr = 0x13,
    Egg = 0x14,
    MoredhelMagician = 0x15,
    BlackSlayer = 0x16,
    Nighthawk = 0x17,
    Rogue = 0x18
};

enum class Actor
{
    Locklear = 1,
    Gorath = 2,
    Owyn = 3,
    Pug = 4,
    James = 5,
    Patrus = 6,
    NavonDuSandau = 7,
    UgyneCorvalis = 8,
    SquirePhillip = 24
};

}
