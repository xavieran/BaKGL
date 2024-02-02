#pragma once

#include <string>

#include "bak/types.hpp"
#include "bak/worldClock.hpp"

namespace BAK {

static constexpr float gWorldScale = 100.;

static constexpr float gTileSize = 64000.;
static constexpr auto gOffsetScale = 0x640; // (1600)

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

static constexpr auto Locklear = CharIndex{0};
static constexpr auto Gorath = CharIndex{1};
static constexpr auto Owyn = CharIndex{2};
static constexpr auto Pug = CharIndex{3};
static constexpr auto James = CharIndex{4};
static constexpr auto Patrus = CharIndex{5};

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

namespace Times {
static constexpr auto HalfHour = Time{0x384};
static constexpr auto OneHour = Time{0x708};
static constexpr auto EightHours = Time{0x3840};
static constexpr auto TwelveHours = Time{0x5460};
static constexpr auto ThirteenHours = Time{0x5b68};
static constexpr auto SeventeenHours = Time{0x7788};
static constexpr auto EighteenHours = Time{0x7e90};
static constexpr auto OneDay = Time{0xa8c0};
}

}
