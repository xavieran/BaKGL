#include "bak/model.hpp"

#include "bak/combatModel.hpp"
#include "bak/dataTags.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/ostream.hpp"
#include "com/logger.hpp"

#include <set>
#include <unordered_map>

int main(int argc, char** argv)
{
    const auto& logger = Logging::LogState::GetLogger("main");
    Logging::LogState::SetLevel(Logging::LogLevel::Spam);
    Logging::LogState::Disable("PackedFileDataProvider");
    
    std::string tbl{argv[1]};
    auto tblBuf = BAK::FileBufferFactory::Get().CreateDataBuffer(tbl);

    logger.Info() << "Loading TBL:" << tbl << std::endl;
    auto models = BAK::LoadTBL(tblBuf);
    for (unsigned i = 0; i < models.size(); i++)
    {
        logger.Info() << "Model #" << i << " " << models[i].mName << "\n";
        if (models[i].mEntityType == 0 && models[i].mSprite > 0)
        {
            logger.Info() << "Combatant Model #" << i << " " << models[i].mName << "\n";
            BAK::CombatModel{models[i]};
        }
    }

    return 0;
}
