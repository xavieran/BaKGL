#include "bak/gameData.hpp"

#include "bak/save/party.hpp"
#include "bak/save/world.hpp"

#include "bak/constants.hpp"
#include "bak/fileBufferFactory.hpp"
#include "bak/resourceNames.hpp"
#include "bak/types.hpp"

#include "com/ostream.hpp"

namespace BAK {

GameData::GameData(const std::string& save)
:
    mBuffer{FileBufferFactory::Get().CreateSaveBuffer(save)},
    mLogger{Logging::LogState::GetLogger("GameData")},
    mName{LoadSaveName(mBuffer)},
    mChapter{LoadChapter(mBuffer)},
    mMapLocation{LoadMapLocation(mBuffer)},
    mLocation{LoadLocation(mBuffer)},
    mTime{LoadWorldTime(mBuffer)},
    mParty{LoadParty(mBuffer)}
{
    mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;
    LoadChapterOffsetP(mBuffer);
}

}
