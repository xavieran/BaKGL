#pragma once

#include "bak/types.hpp"
#include "bak/combatModel.hpp"

#include "graphics/renderData.hpp"

#include <optional>
#include <utility>
#include <vector>

namespace BAK {
struct Model;
}

namespace Game {

struct AnimationRequest
{
    BAK::AnimationType mAnimation;
    BAK::Direction mDirection;

    bool operator==(const AnimationRequest& other) const = default;
};

}

namespace std {

template<> struct hash<Game::AnimationRequest>
{
	std::size_t operator()(const Game::AnimationRequest& t) const noexcept
	{
		return std::hash<std::size_t>{}(
            std::to_underlying(t.mAnimation) | (std::to_underlying(t.mDirection) << 8));
	}
};

}

namespace Game {

struct AnimationOffset
{
    std::size_t mOffset;
    std::size_t mFrames;
};


struct CombatModel
{
    Graphics::RenderData mCombatModels{};
    std::unordered_map<AnimationRequest, AnimationOffset> offsetMap;
};

class CombatModelLoader
{
    static constexpr auto sCombatModels = "COMBAT.TBL";
public:

    CombatModelLoader();

    void LoadMonsterSprites(BAK::MonsterIndex m);

    std::vector<std::optional<BAK::CombatModel>> mCombatModels{};
};

}

