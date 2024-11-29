#pragma once

#include "bak/types.hpp"
#include "bak/combatModel.hpp"

#include "graphics/meshObject.hpp"
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
	std::size_t operator()(const Game::AnimationRequest& t) const noexcept;
};

}

namespace Game {

struct AnimationOffset
{
    std::size_t mOffset;
    std::size_t mFrames;
};

struct CombatModelData
{
    Graphics::RenderData mRenderData{};
    std::unordered_map<AnimationRequest, AnimationOffset> mOffsetMap;
    std::vector<Graphics::MeshObjectStorage::OffsetAndLength> mObjectDrawData;
};

class CombatModelLoader
{
    static constexpr auto sCombatModels = "COMBAT.TBL";
public:

    CombatModelLoader();

    void LoadMonsterSprites(BAK::MonsterIndex m);

    std::vector<std::optional<BAK::CombatModel>> mCombatModels{};
    std::vector<std::optional<CombatModelData>> mCombatModelDatas{};
};

}

