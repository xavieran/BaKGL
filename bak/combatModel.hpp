#pragma once

#include <cstdint>
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

private:
    std::vector<std::array<CombatAnimation, 5>> mCombatAnimations;
};

}
