#pragma once

#include "bak/condition.hpp"

#include <array>
#include <cstdint>
#include <utility>

namespace BAK {
namespace SaveOffsets {

static constexpr auto sCharacterCount = 6;
static constexpr auto sChapterOffset = 0x5a;
static constexpr auto sMapPositionOffset = 0x5c;
static constexpr auto sGoldOffset = 0x66;
static constexpr auto sTimeOffset = 0x6a;
static constexpr auto sLocationOffset = 0x76;

static constexpr auto sCharacterNameOffset    = 0x9f;
static constexpr auto sCharacterNameLength    = 10;

static constexpr auto sCharacterSkillOffset   = 0xdb;
static constexpr auto sCharacterSkillLength   = 5 * 16 + 8 + 7;
static constexpr auto sActiveCharactersOffset = 0x315;
static constexpr auto sCharacterStatusOffset  = 0x330;
static constexpr auto sCharacterSkillAffectorOffset  = 0x35a;

static constexpr auto sTimeExpiringEventRecordOffset = 0x616;
static constexpr auto sActiveSpells = 0x6b8;

static constexpr auto sPantathiansEventFlag = 0x1ed4;

// 0x131F in the game
static constexpr auto sCombatEntityListCount  = 700;
static constexpr auto sCombatEntityListOffset = 0x1383;

static constexpr auto sCombatWorldLocationsOffset = 0x4fab;
static constexpr auto sCombatWorldLocationsCount = 1400;

// 0x312E5 in the game
static constexpr auto sCombatantGridLocationsOffset = 0x31349;
static constexpr auto sCombatantGridLocationsCount = 1699;

static constexpr auto sCombatStatsOffset = 0x914b;
static constexpr auto sCombatStatsCount = 1699;

static constexpr auto sCharacterInventoryOffset = 0x3a804;
static constexpr auto sCharacterInventoryLength = 0x70;

static constexpr auto sPartyKeyInventoryOffset = 0x3aaa4;

inline constexpr std::array<std::pair<unsigned, unsigned>, 13> sZoneContainerOffsets = {{
    {0x3ab4f, 15},
    {0x3b621, 36},
    {0x3be55, 25},
    {0x3c55f, 54},
    {0x3d0b4, 65},
    {0x3dc07, 63},
    {0x3e708, 131},
    {0x3f8b2, 115},
    {0x40c97, 67},
    {0x416b7, 110},
    {0x42868, 25},
    {0x43012, 30},
    {0x4378f, 60}
}};

static constexpr auto sShopsCount  = 98;
static constexpr auto sShopsOffset = 0x443c9;

static constexpr auto sCombatInventoryCount  = 1734;
static constexpr auto sCombatInventoryOffset = 0x46053;

constexpr unsigned GetCharacterNameOffset(unsigned c) { return c * sCharacterNameLength + sCharacterNameOffset; }
constexpr unsigned GetCharacterSkillOffset(unsigned c) { return c * sCharacterSkillLength + sCharacterSkillOffset; }
constexpr unsigned GetCharacterInventoryOffset(unsigned c) { return c * sCharacterInventoryLength + sCharacterInventoryOffset; }
constexpr unsigned GetCharacterConditionOffset(unsigned c) { return c * Conditions::sNumConditions + sCharacterStatusOffset; }
constexpr unsigned GetCharacterAffectorsOffset(unsigned c) { return (c * 8 * 14) + sCharacterSkillAffectorOffset; }

}
}
