#pragma once

#include "bak/combatModel.hpp"

#include <vector>

namespace BAK {
struct Model;
}

namespace Graphics {
class SpriteManager;
}

namespace Game {

class CombatModelLoader
{
    static constexpr auto sCombatModels = "COMBAT.TBL";
public:
    CombatModelLoader(Graphics::SpriteManager&);


    std::vector<BAK::CombatModel> mCombatModels{};
    Graphics::SpriteManager& mSpriteManager;
};

}
