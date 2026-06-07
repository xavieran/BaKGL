#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace BAK {

struct Model;

enum class AnimationType : std::uint8_t
{
    Idle,
    Dead,
    Slash,
    Thrust,
    ParryLow,
    ParryHigh,
    Ranged,
    RangedCast,
    StaticCast
};

std::string_view ToString(AnimationType);

enum class Direction : std::uint8_t
{
    South,
    SouthEast,
    East,
    NorthEast,
    North,
    // These are always created by mirroring their opposites
    NorthWest,
    West,
    SouthWest
};

std::string_view ToString(Direction direction);
const std::vector<BAK::Direction>& GetDirections(bool granular);

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
