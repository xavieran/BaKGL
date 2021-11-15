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
#include "bak/skills.hpp"
#include "bak/types.hpp"
#include "bak/worldClock.hpp"

#include "com/logger.hpp"

#include "xbak/FileBuffer.h"

#include <vector>
#include <memory>

namespace BAK {

struct Location
{
    unsigned mZone;
    glm::vec<2, unsigned> mTile;
    GamePositionAndHeading mLocation;
};

class GameData
{   
public:
/*
 *
 * Locklear, Gorath, Owyn, Pug, James, Patrus
 *
 * Characters Start @0xdb
 * 0x9fd -> Indicates unseen character stat improvement (bit flags)
 * * 0x011a0 - Character Inventory Offsets Start???
 * 0x01383 - Combat Entity List Start
 * 0x039cb - Combat Entity List Start
 *
 * 0x04fb0 - Combat locations
 * Note: This is only populated if loaded by a tile
 * 0x52f0 - combat 1 location start
 * c8 a4 00 00 5a 2b 00 00 00 80 00 03 00
 * X loc       yloc       rotation State (dead/alive/invislbe?)
 * 0x0913e - Events End??
 * @10 bytes each => 1678 combat locs?
 *
 * 0x0914b - Combat Stats Start 
 * 0x3070a - Combat Stats End
 *
 * 0x31340 - Combats Start
 * 
 * 0x3a7f0 - Character Inventory Start
 * 0x3aaf0 - Character Invenorty Start ???
 *
 * 0x3b621 - Start of Containers
 * 0x44c37 - End of Containers ???
 * 0x44cc9 - Start of Shops
 * 0x46043 End of shops
 * 0x46053 - Combat Inventory Start
 * 0x51720 - Combat Inventory End (1773)
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
    static constexpr auto sCharacterSkillOffset   = 0xdb; // -> 0x315
    static constexpr auto sActiveCharactersOffset = 0x315; // -> 0x319
    static constexpr auto sCharacterStatusOffset  = 0x330;
    static constexpr auto sCharacterPotionOffset  = 0x350;

    // Single bit indicators for event state tracking 
    // In the code this offset is 0x440a in the game -> diff of 0x3d28
    static constexpr auto sGameEventRecordOffset = 0x6e2; // -> 0xadc
    static constexpr auto sGameComplexEventRecordOffset = 0xb09; // -> 0xadc

    static constexpr auto sConversationChoiceMarkedFlag = 0x1d4c;
    static constexpr auto sConversationOptionInhibitedFlag = 0x1a2c;
    // Based on disassembly this may be the state of doors (open/closed)
    static constexpr auto sDoorFlag = 0x1b58;

    static constexpr auto sSkillSelectedEventFlag = 0x1856;
    static constexpr auto sSkillImprovementEventFlag= 0x18ce;

    static constexpr auto sLockHasBeenSeenFlag = 0x1c5c;

    static constexpr auto sCombatEntityListCount  = 700;
    static constexpr auto sCombatEntityListOffset = 0x1383;

    static constexpr auto sCharacterInventoryOffset = 0x3a804; // -> 3aa4b
    static constexpr auto sPartyKeyInventoryOffset = 0x3aaa4;

    static constexpr auto sZone1ContainerOffset = 0x3b631;
    static constexpr auto sZone2ContainerOffset = 0x3be55;
    static constexpr auto sZone3ContainerOffset = 0x3c55f;
    static constexpr auto sZone4ContainerOffset = 0x3d0b4;
    static constexpr auto sZone5ContainerOffset = 0x3dc07;
    static constexpr auto sZone6ContainerOffset = 0x3e708;
    static constexpr auto sZone7ContainerOffset = 0x3f8b2;
    static constexpr auto sZone8ContainerOffset = 0x40c97;
    static constexpr auto sZone9ContainerOffset = 0x416b7;
    static constexpr auto sZoneAContainerOffset = 0x42868;
    static constexpr auto sZoneBContainerOffset = 0x43012;
    static constexpr auto sZoneCContainerOffset = 0x4378f;
    
    static constexpr auto sShopsCount  = 98;
    static constexpr auto sShopsOffset = 0x443c9;

    static constexpr auto sCombatInventoryCount  = 1734;
    static constexpr auto sCombatInventoryOffset = 0x46053;

    GameData(const std::string& save);

    void Save(const std::string& saveName)
    {
        ASSERT(saveName.size() < 30);
        mBuffer.Seek(0);
        mBuffer.PutString(saveName);

        auto saveFile = std::ofstream{
            saveName,
            std::ios::binary | std::ios::out};
        mBuffer.Save(saveFile);
    }

    std::pair<unsigned, unsigned> CalculateComplexEventOffset(unsigned eventPtr) const;
    std::pair<unsigned, unsigned> CalculateEventOffset(unsigned eventPtr) const;

    void SetBitValueAt(unsigned byteOffset, unsigned bitOffset, unsigned value);
    void SetEventFlag(unsigned eventPtr, unsigned value);
    void SetEventFlagTrue (unsigned eventPtr);
    void SetEventFlagFalse(unsigned eventPtr);

    void SetEventDialogAction(const SetFlag& setFlag);

    unsigned ReadBitValueAt(unsigned byteOffset, unsigned bitOffset) const;
    unsigned ReadEvent(unsigned eventPtr) const;
    bool ReadEventBool(unsigned eventPtr) const;

    bool ReadSkillSelected(unsigned character, unsigned skill) const;
    bool ReadSkillUnseenImprovement(unsigned character, unsigned skill) const;
    void ClearUnseenImprovements(unsigned character);

    // Called by checkBlockTriggered, checkTownTriggered, checkBackgroundTriggered, checkZoneTriggered,
    // doEnableEncounter, doDialogEncounter, doDisableEncounter, doSoundEncounter
    bool CheckActive(
        const Encounter::Encounter& encounter,
        ZoneNumber zone) const;

    void SetPostDialogEventFlags(const Encounter::Encounter& encounter);
    
    // Background and Town
    void SetPostGDSEventFlags(const Encounter::Encounter& encounter);
    
    // Used by Block, Disable, Enable, Sound, Zone
    void SetPostEnableOrDisableEventFlags(
        const Encounter::Encounter& encounter,
        ZoneNumber zone);

    // For each encounter in every zone there is a unique enabled/disabled flag.
    // This is reset every time a new chapter is loaded (I think);
    unsigned CalculateUniqueEncounterStateFlagOffset(
        ZoneNumber zone, 
        std::uint8_t tileIndex,
        std::uint8_t encounterIndex) const;
    
    // 1450 is "recently encountered this encounter"
    // should be cleared when we move to a new tile
    // (or it will inhibit the events of the new tile)
    unsigned CalculateRecentEncounterStateFlag(
        std::uint8_t encounterIndex) const;

    void ClearTileRecentEncounters();
    
    bool ReadConversationItemClicked(unsigned eventPtr) const;
    void SetConversationItemClicked(unsigned eventPtr);
    bool CheckConversationOptionInhibited(unsigned eventPtr);

    void SetLockHasBeenSeen(unsigned lockIndex);
    bool CheckLockHasBeenSeen(unsigned lockIndex);
    
    /* ************* LOAD Game STATE ***************** */
    Party LoadParty();
    std::vector<Character> LoadCharacters();
    Conditions LoadConditions(unsigned character);
    unsigned LoadChapter();
    Royals LoadGold();
    std::vector<CharIndex> LoadActiveCharacters();
    Location LoadLocation();
    WorldClock LoadWorldTime();

    Inventory LoadInventory(unsigned offset);
    std::vector<InventoryItem> LoadItems(unsigned itemCount, unsigned capacity);
    
    LockStats LoadLock();

    ShopStats LoadShop();
    std::vector<GDSContainer> LoadShops();
    std::vector<Container> LoadContainers(unsigned zone);

    // Probablynot chapter offsets.. ?
    void LoadChapterOffsetP();
    void LoadCombatEntityLists();
    void LoadCombatStats(unsigned offset, unsigned num);
    void LoadCombatInventories(unsigned offset, unsigned number);

    std::string LoadSaveName();

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
