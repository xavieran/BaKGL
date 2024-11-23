#include "game/combatModelLoader.hpp"

#include "bak/combatModel.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/image.hpp"
#include "bak/imageStore.hpp"
#include "bak/model.hpp"
#include "bak/monster.hpp"
#include "bak/palette.hpp"
#include "bak/resourceNames.hpp"
#include "bak/textureFactory.hpp"
#include "bak/worldFactory.hpp"

#include "com/logger.hpp"
#include "com/string.hpp"

#include "graphics/texture.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Game {

CombatModelLoader::CombatModelLoader()
{
    const auto& logger = Logging::Logger("CombatModelLoader");
    auto tblBuf = BAK::FileBufferFactory::Get().CreateDataBuffer(sCombatModels);
    auto models = BAK::LoadTBL(tblBuf);
    for (unsigned i = 0; i < models.size(); i++)
    {
        if (models[i].mEntityType == 0 && models[i].mSprite > 0)
        {
            logger.Info() << "Loaded Combatant Model #" << i << " " << models[i].mName << "\n";
            mCombatModels.emplace_back(BAK::CombatModel{models[i]});
        }
        else
        {
            mCombatModels.emplace_back(std::nullopt);
        }
    }
}

void CombatModelLoader::LoadMonsterSprites(BAK::MonsterIndex m)
{
    const auto& mnames = BAK::MonsterNames::Get();
    auto textureStore = Graphics::TextureStore{};
    auto monster = mnames.GetMonster(m);
    auto prefix = ToUpper(monster.mPrefix);
    if (prefix.empty())
    {
        return;
    }

    auto pal = BAK::Palette{BAK::ZoneLabel{1}.GetPalette()};
    if (monster.mColorSwap <= 9)
    {
        auto ss = std::stringstream{};
        ss << "CS";
        ss << +monster.mColorSwap << ".DAT";
        const auto cs = BAK::ColorSwap{ss.str()};
        pal = BAK::Palette{pal, cs};
    }

    auto LoadImages = [&](auto suffix)
    {
        std::stringstream ss{prefix};
        ss << suffix << ".BMX";
        auto fb = BAK::FileBufferFactory::Get().CreateDataBuffer(ss.str());
        const auto images = BAK::LoadImages(fb);
        BAK::TextureFactory::AddToTextureStore(textureStore, images, pal);
    };

    std::vector<std::size_t> offsets{0};
    {
        LoadImages(monster.mSuffix0);
        offsets.emplace_back(textureStore.size());
        LoadImages(monster.mSuffix1);
        offsets.emplace_back(textureStore.size());
        LoadImages(monster.mSuffix2);
    }

    assert(mCombatModels[m.mValue]);
    const auto& model = *mCombatModels[m.mValue];
    std::vector<BAK::ZoneItem> objects{};
    std::unordered_map<AnimationRequest, AnimationOffset> offsetMap{};
    using enum BAK::Direction;
    for (const auto& animType : model.GetSupportedAnimations())
    {
        const bool truncated = model.GetDirections(animType) == 3;
        auto directions = {South, East, North};
        for (const auto& direction : directions)
        {
            const auto& animation = model.GetAnimation(animType, direction);
            offsetMap.emplace(
                AnimationRequest{animType, direction},
                AnimationOffset{objects.size(), animation.mImageIndices.size()});
            for (const auto& index : animation.mImageIndices)
            {
                assert(animation.mSpriteFileIndex < offsets.size());
                const auto fullOffset = offsets[animation.mSpriteFileIndex] + index;
                objects.emplace_back(BAK::ZoneItem(fullOffset, textureStore.GetTexture(fullOffset)));
            }
        }
    }
}

}

