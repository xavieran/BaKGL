#include "game/combatModelLoader.hpp"

#include "bak/combatModel.hpp"
#include "bak/model.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

namespace Game {

CombatModelLoader::CombatModelLoader(
    Graphics::SpriteManager& spriteManager)
:
    mSpriteManager{spriteManager}
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
    }
}

}

