#pragma once

#include <glm/glm.hpp>

#include "bak/constants.hpp"
#include "bak/character.hpp"
#include "bak/condition.hpp"
#include "bak/container.hpp"
#include "bak/dialogAction.hpp"
#include "bak/encounter/encounter.hpp"
#include "bak/money.hpp"
#include "bak/party.hpp"
#include "bak/resourceNames.hpp"
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

struct Location
{
    unsigned mZone;
    glm::vec<2, unsigned> mTile;
    GamePositionAndHeading mLocation;
};

std::ostream& operator<<(std::ostream& os, const Location&);

class GameData
{   
public:
/*
 *
 * 0x04fb0 - Combat locations
 * Note: This is only populated if loaded by a tile
 * 0x52f0 - combat 1 location start
 * c8 a4 00 00 5a 2b 00 00 00 80 00 03 00
 * X loc       yloc       rotation State (dead/alive/invislbe?)
 * 0x0913e - Events End??
 * @10 bytes each => 1678 combat locs?
 *
 */

 /* 
  * Combat starts 31340 - 12 2 06 - 12 sprite 2,6 grid location
  * Combat Zone 1 1
  * Inventory #3 Combat #1 Person #0 0x460b7
  * Combat Stats #3 0x9268 (bbe0)? 
  */

    // Offset refers to the raw offset in the save file
    // "Flag" refers to the flag used in the code that is 
    // processed by "CalculateEventOffset" or "CalculateComplexEventOffset"
    // to generate a raw offset
    
    static constexpr auto sCharacterCount = 6;
    static constexpr auto sChapterOffset = 0x5a; // -> 5c
    static constexpr auto sGoldOffset = 0x66; // -> 6a
    static constexpr auto sTimeOffset = 0x6a; // -> 0x72
    static constexpr auto sLocationOffset = 0x76; // -> 0x88

    static constexpr auto sCharacterNameOffset    = 0x9f; // -> 0xdb
    static constexpr auto sCharacterNameLength    = 10;

    static constexpr auto sCharacterSkillOffset   = 0xdb; // -> 0x315
    static constexpr auto sCharacterSkillLength   = 5 * 16 + 8 + 7;
    static constexpr auto sActiveCharactersOffset = 0x315; // -> 0x319
                                                           //
    static constexpr auto sCharacterStatusOffset  = 0x330;
    static constexpr auto sCharacterSkillAffectorOffset  = 0x35a;

    static constexpr auto sTimeExpiringEventRecordOffset = 0x616;
    static constexpr auto sActiveSpells = 0x6b8;

    static constexpr auto sPantathiansEventFlag = 0x1ed4;

    static constexpr auto sCombatEntityListCount  = 700;
    static constexpr auto sCombatEntityListOffset = 0x1383;

    static constexpr auto sCombatWorldLocationsOffset = 0x4fab;
    static constexpr auto sCombatWorldLocationsCount = 1400;

    static constexpr auto sCombatSkillsListOffset = 0x914b;
    static constexpr auto sCombatSkillsListCount  = 1698;

    static constexpr auto sCombatGridLocationsOffset = 0x31349;
    static constexpr auto sCombatGridLocationsCount = 1699;

    static constexpr auto sCharacterInventoryOffset = 0x3a804; // -> 3aa4b
    static constexpr auto sCharacterInventoryLength = 0x70; // -> 3aa4b

    static constexpr auto sPartyKeyInventoryOffset = 0x3aaa4;

    static constexpr std::array<std::pair<unsigned, unsigned>, 13> sZoneContainerOffsets = {
        std::make_pair(0x3ab4f, 15), // cheat chests and debug inventories...
        {0x3b621, 36},
        {0x3be55, 25},
        {0x3c55f, 54},
        {0x3d0b4, 65},
        {0x3dc07, 63},
        {0x3e708, 131},
        {0x3f8b2, 115},
        {0x40c97, 67},
        {0x416b7, 110},
        {0x42868, 25},
        {0x43012, 30},
        {0x4378f, 60}
    };

    static constexpr auto sShopsCount  = 98;
    static constexpr auto sShopsOffset = 0x443c9;

    static constexpr auto sCombatInventoryCount  = 1734;
    static constexpr auto sCombatInventoryOffset = 0x46053;

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

        // Location
        mBuffer.Seek(sLocationOffset);
        mBuffer.PutUint8(mLocation.mZone);
        mBuffer.PutUint8(mLocation.mTile.x);
        mBuffer.PutUint8(mLocation.mTile.y);
        mBuffer.PutUint32LE(mLocation.mLocation.mPosition.x);
        mBuffer.PutUint32LE(mLocation.mLocation.mPosition.y);
        mBuffer.Skip(5);
        mBuffer.PutUint16LE(mLocation.mLocation.mHeading);


        mLogger.Info() << "Saving game to: " << savePath << std::endl;
        auto saveFile = std::ofstream{
            savePath,
            std::ios::binary | std::ios::out};
        mBuffer.Save(saveFile);
    }

    FileBuffer& GetFileBuffer() { return mBuffer; }
    std::vector<TimeExpiringState> LoadTimeExpiringState();
    SpellState LoadSpells();

    std::vector<SkillAffector> GetCharacterSkillAffectors(CharIndex character);

    /* ************* LOAD Game STATE ***************** */
    static constexpr unsigned GetCharacterNameOffset(unsigned c) { return c * sCharacterNameLength + sCharacterNameOffset; }
    static constexpr unsigned GetCharacterSkillOffset(unsigned c) { return c * sCharacterSkillLength + sCharacterSkillOffset; }
    static constexpr unsigned GetCharacterInventoryOffset(unsigned c) { return c * sCharacterInventoryLength + sCharacterInventoryOffset; }
    static constexpr unsigned GetCharacterConditionOffset(unsigned c) { return c * Conditions::sNumConditions + sCharacterStatusOffset; }
    static constexpr unsigned GetCharacterAffectorsOffset(unsigned c) { return (c * 8 * 14) + sCharacterSkillAffectorOffset; }

    Party LoadParty();
    std::vector<Character> LoadCharacters();
    Conditions LoadConditions(unsigned character);
    unsigned LoadChapter();
    Royals LoadGold();
    std::vector<CharIndex> LoadActiveCharacters();
    Location LoadLocation();
    WorldClock LoadWorldTime();

    Inventory LoadCharacterInventory(unsigned offset);
    
    std::vector<GenericContainer> LoadShops();
    std::vector<GenericContainer> LoadContainers(unsigned zone);
    std::vector<GenericContainer> LoadCombatInventories();

    // Probably not chapter offsets.. ?
    void LoadChapterOffsetP();
    void LoadCombatEntityLists();
    void LoadCombatGridLocations();
    void LoadCombatWorldLocations();
    void LoadCombatStats(unsigned offset, unsigned num);
    void LoadCombatClickedTimes();

    mutable FileBuffer mBuffer;
    Logging::Logger mLogger;

    const std::string mName;
    ObjectIndex mObjects;
    Chapter mChapter;
    Location mLocation;
    WorldClock mTime;
    Party mParty;
};

}
