#include "bak/combatModel.hpp"

#include "bak/model.hpp"

#include "com/logger.hpp"

#include <cassert>
#include <utility>

namespace BAK {

std::string_view ToString(AnimationType animationType)
{
    using enum AnimationType;
    switch (animationType)
    {
        case Idle: return "Idle";
        case Dead: return "Dead";
        case Slash: return "Slash";
        case Thrust: return "Thrust";
        case ParryLow: return "ParryLow";
        case ParryHigh: return "ParryHigh";
        case Ranged: return "Ranged";
        case RangedCast: return "RangedCast";
        case StaticCast: return "StaticCast";
    }
    std::unreachable();
}

std::string_view ToString(Direction direction)
{
    using enum Direction;
    switch (direction)
    {
        case South: return "South";
        case SouthEast: return "SouthEast";
        case East: return "East";
        case NorthEast: return "NorthEast";
        case North: return "North";
        case NorthWest: return "NorthWest";
        case West: return "West";
        case SouthWest: return "SouthWest";
    }
    std::unreachable();
}



CombatModel::CombatModel(const Model& model)
:
    mCombatAnimations{}
{
    const auto& logger = Logging::Logger("CombatModel");

    assert(model.mComponents.size() == 1);
    const auto& meshes = model.mComponents.back().mMeshes;
    std::uint8_t spriteFileIndex = 0;
    std::uint8_t lastSpriteIndex = 0;
    logger.Debug() << "Have Meshes size: " << meshes.size() << "\n";
    for (std::uint8_t i = 0; i < meshes.size(); i++)
    {
        auto& animations = mCombatAnimations.emplace_back(std::array<CombatAnimation, 5>{});
        const auto& faceOptions = meshes[i].mFaceOptions;
        logger.Debug() << "Handle mesh " << +i << " SF: " << +spriteFileIndex << " LSI: " 
            << +lastSpriteIndex << " FOs: " << faceOptions.size() << "\n";

        if (lastSpriteIndex > faceOptions.front().mEdgeCount)
        {
            spriteFileIndex++;
            logger.Debug() << "Next sprite is: " << +faceOptions.front().mEdgeCount << " last was: "
                << +lastSpriteIndex << " SFI is now: " << +spriteFileIndex << "\n";
        }

        const auto animationType = static_cast<AnimationType>(i);
        //const auto animationCount = GetAnimationCount(animationType);
        //const auto directionCount = faceOptions.size() / animationCount;
        logger.Debug() << "AnimType: " << ToString(animationType) << " FaceOptions size: " << faceOptions.size() << " mod5: " 
            << faceOptions.size() % 5 << " mod3: " << faceOptions.size() % 3 << std::endl;

        auto frameCount = faceOptions.size();
        if (!((faceOptions.size() % 5 == 0) || (faceOptions.size() % 3 == 0)))
        {
            if (frameCount == 7)
            {
                frameCount--;
            }
            else
            {
                frameCount++;
            }
            logger.Warn() << "FaceOptions size is not divisible by 5 or 3 FC now: " << frameCount << std::endl;
        }

        //assert((frameCount % 5 == 0) || (frameCount % 3 == 0));
        if (!((frameCount % 5 == 0) || (frameCount % 3 == 0)))
        {
            logger.Warn() << "FC is now: " << frameCount << std::endl;
            exit(1);
        }
        const auto directionCount = frameCount == 0 ? 5 : 3;
        const auto animationCount = frameCount == 0
            ? faceOptions.size() / 5
            : faceOptions.size() / 3;
        using enum Direction;
        const auto& directions = directionCount == 5
            ? std::vector<Direction>{South, SouthEast, East, NorthEast, North}
            : std::vector<Direction>{South, East, North};
        logger.Debug() << "AnimationCount: " << +animationCount << " DirCount: " << +directionCount << "\n";

        unsigned fo = 0;
        for (const auto& direction : directions)
        {
            animations[std::to_underlying(direction)] = CombatAnimation{spriteFileIndex, std::vector<std::uint8_t>{}};
            logger.Debug() << "  Handling dir: " << +std::to_underlying(direction) << " " << ToString(direction) << "\n";
            auto& imageIndices = animations[std::to_underlying(direction)].mImageIndices;
            for (unsigned j = 0; j < animationCount; j++)
            {
                if (fo >= faceOptions.size())
                {
                    logger.Warn() << "Exceeded face options" << std::endl;
                    continue;
                }
                const auto spriteIndex = faceOptions[fo++].mEdgeCount;
                imageIndices.emplace_back(spriteIndex);
                lastSpriteIndex = spriteIndex;
                logger.Debug() << "    Placing SI: " << +spriteIndex << "\n";
            }
        }
    }
}

CombatAnimation CombatModel::GetAnimation(AnimationType animationType, Direction direction) const
{
    auto animIndex = std::to_underlying(animationType);
    assert(animIndex < mCombatAnimations.size());

    const auto& byDirection = mCombatAnimations[animIndex];
    auto dirIndex = std::to_underlying(direction);
    assert(dirIndex < byDirection.size());
    return byDirection[dirIndex];
}

}
