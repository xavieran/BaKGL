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

enum class EncounterType : std::uint16_t
{
    Unknown1 = 0, // DEF_BKGR.DAT?? teleport??
    Combat = 1,  // DEF_COMB.DAT
    Unknown2 = 2,  // ???
    Dialog = 3,  // ???
    Unknown3 = 4,  // ???
    Sound = 5,  // DEF_SOUN.DAT
    Town = 6,  // DEF_TOWN.DAT
    Trap = 7,  // DEF_TRAP.DAT
    Transition = 8,  // DEF_??.DAT
    Unknown4 = 9,  // DEF_DISA.DAT
    Unknown5 = 0xa,  // DEF_ENAB.DAT
    Unknown6 = 0xb  // DEF_BLOC.DAT
};

std::string EncounterTypeToString(EncounterType t)
{
    switch (t)
    {
    case EncounterType::Unknown1: return "Unknown1";
    case EncounterType::Combat: return "Combat";
    case EncounterType::Unknown2: return "Unknown2";
    case EncounterType::Dialog: return "Dialog";
    case EncounterType::Unknown3: return "Unknown3";
    case EncounterType::Sound: return "Sound";
    case EncounterType::Town: return "Town";
    case EncounterType::Trap: return "Trap";
    case EncounterType::Transition: return "Transition";
    default: return "Unknown";
    }
}

}
