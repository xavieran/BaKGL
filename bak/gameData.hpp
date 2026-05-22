#pragma once

#include <glm/glm.hpp>

#include "bak/coordinates.hpp"
#include "bak/party.hpp"
#include "bak/saveManager.hpp"
#include "bak/skills.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include "com/logger.hpp"

#include "bak/fileBufferFactory.hpp"

namespace BAK {

class GameData
{   
public:
    GameData();
    GameData(const std::string& save);

    bool IsLoaded() const { return mIsLoaded; }

    void Load(const std::string& save);

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
    FileBuffer& GetFileBuffer() const { return mBuffer; }

    mutable FileBuffer mBuffer;
    Logging::Logger mLogger;

    std::string mName;
    Chapter mChapter;
    MapLocation mMapLocation;
    Location mLocation;
    WorldClock mTime;
    Party mParty;
    bool mIsLoaded{false};
};

}
