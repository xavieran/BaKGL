#pragma once

#include <glm/glm.hpp>

#include "bak/character.hpp"
#include "bak/condition.hpp"
#include "bak/container.hpp"
#include "bak/combat.hpp"
#include "bak/party.hpp"
#include "bak/save/saveOffsets.hpp"
#include "bak/saveManager.hpp"
#include "bak/skills.hpp"
#include "bak/spells.hpp"
#include "bak/timeExpiringState.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

#include <vector>
#include <memory>

namespace BAK {

class GameData
{   
public:
    GameData(const std::string& save);

    void Save(const SaveFile& saveFile)
    {
        Save(saveFile.mName, saveFile.mPath);
    }

    void Save(
        const std::string& saveName,
        const std::string& savePath)
    {
        ASSERT(saveName.size() < 30);
        mBuffer.Seek(0);
        mBuffer.PutString(saveName);

        mLogger.Info() << "Saving game to: " << savePath << std::endl;
        auto saveFile = std::ofstream{
            savePath,
            std::ios::binary | std::ios::out};
        mBuffer.Save(saveFile);
    }

    FileBuffer& GetFileBuffer() { return mBuffer; }

    mutable FileBuffer mBuffer;
    Logging::Logger mLogger;

    const std::string mName;
    Chapter mChapter;
    MapLocation mMapLocation;
    Location mLocation;
    WorldClock mTime;
    Party mParty;
};

}
