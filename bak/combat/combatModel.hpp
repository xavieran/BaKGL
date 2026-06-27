#pragma once

#include "bak/coordinates.hpp"

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace BAK {

struct Model;

enum class AnimationType : std::uint8_t
{
    Idle = 0,
    Dead = 1,
    Slash = 2,
    Thrust = 3,
    ParryLow = 4,
    ParryHigh = 5,
    Ranged = 6,
    RangedCast = 7,
    StaticCast = 8
};

std::string_view ToString(AnimationType);
std::string_view ToString(Direction direction);
const std::vector<BAK::Direction>& GetDirections(bool granular);
BAK::Direction ToSpriteDirection(Direction direction);

class CombatAnimation
{
public:
    std::uint8_t mSpriteFileIndex;
    std::vector<std::uint8_t> mImageIndices;
};

class CombatModel
{
public:
    CombatModel(const Model&);
    CombatAnimation GetAnimation(AnimationType, Direction) const;
    const std::vector<Direction>& GetDirections(AnimationType) const;
    const std::vector<AnimationType>& GetSupportedAnimations() const;

private:
    using Directions = std::array<CombatAnimation, 5>;
    std::vector<AnimationType> mAnimationTypes;
    std::vector<Directions> mCombatAnimations;
};

}
