#pragma once

#include "com/strongType.hpp"

#include <cstdint>

namespace BAK {

using Chapter = unsigned;
using EntityIndex = StrongType<unsigned, struct EntityIndexTag>;
using ChoiceIndex = StrongType<unsigned, struct ChoiceIndexTag>;
using SongIndex = std::uint16_t;
using AdsSceneIndex = std::uint16_t;
using ItemIndex = StrongType<unsigned, struct ItemIndexTag>;
using ZoneNumber = StrongType<unsigned, struct ZoneNumberTag>;
using ZoneTransitionIndex = StrongType<unsigned, struct ZoneTransitionIndexTag>;

}
