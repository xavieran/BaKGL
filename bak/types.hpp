#pragma once

#include "com/strongType.hpp"

#include <cstdint>

namespace BAK {

using Chapter = Bounded<StrongType<unsigned, struct ChapterTag>, 1, 11>;
using EntityIndex = StrongType<unsigned, struct EntityIndexTag>;
using ChoiceIndex = StrongType<unsigned, struct ChoiceIndexTag>;
using MonsterIndex = StrongType<unsigned, struct MonsterIndexTag>;
using SpellIndex = StrongType<std::uint64_t, struct SpellIndexTag>;
using SongIndex = std::uint16_t;
using AdsSceneIndex = std::uint16_t;
using ItemIndex = StrongType<unsigned, struct ItemIndexTag>;
using ZoneNumber = StrongType<unsigned, struct ZoneNumberTag>;
using ZoneTransitionIndex = StrongType<unsigned, struct ZoneTransitionIndexTag>;
using TeleportIndex = StrongType<unsigned, struct TeleportIndexTag>;

static constexpr auto sMaxActiveCharacters = 3;
static constexpr auto sMaxCharacters = 6;

using ActiveCharIndex = Bounded<
    StrongType<unsigned, struct ActiveCharIndexTag>,
    0, sMaxActiveCharacters>;

using CharIndex = Bounded<
    StrongType<unsigned, struct CharIndexTag>,
    0, sMaxCharacters>;

enum class Loop
{
    Continue,
    Finish
};

}
