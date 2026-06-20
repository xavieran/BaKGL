#include "bak/gameData.hpp"

#include "bak/save/party.hpp"
#include "bak/save/world.hpp"

#include "bak/fileBufferFactory.hpp"
#include "bak/types.hpp"


namespace BAK {

GameData::GameData()
:
    mBuffer{1},
    mLogger{Logging::LogState::GetLogger("GameData")},
    mName{""},
    mChapter{1},
    mMapLocation{{20, 20}, 0},
    mLocation{ZoneNumber{1}, glm::uvec2{0, 0}, GamePositionAndHeading{glm::uvec2{10 * 64000, 15 * 64000}, 0}},
    mTime{Time{0}, Time{0}},
    mParty{
        Royals{1000},
        Inventory{20},
        {},
        std::vector<Character>{
            Character{
                0,
                "None",
                Skills{},
                Spells{{}},
                {},
                uint8_t{0},
                {},
                Conditions{},
                nullptr}},
        std::vector<CharIndex>{CharIndex{0}}}
{
}

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
    mIsLoaded = true;
}

void GameData::Load(const std::string& save)
{
    mLogger.Info() << "Loading save: " << mBuffer.GetString() << std::endl;
    mBuffer = FileBufferFactory::Get().CreateSaveBuffer(save);
    mName = LoadSaveName(mBuffer);
    mChapter = Chapter{LoadChapter(mBuffer)};
    mMapLocation = LoadMapLocation(mBuffer);
    mLocation = LoadLocation(mBuffer);
    mTime = LoadWorldTime(mBuffer);
    mParty = LoadParty(mBuffer);
    LoadChapterOffsetP(mBuffer);
    mIsLoaded = true;
}

}
