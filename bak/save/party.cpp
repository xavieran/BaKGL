#include "bak/save/party.hpp"
#include "bak/save/character.hpp"

#include "bak/constants.hpp"
#include "bak/save/saveOffsets.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

namespace BAK {

Party LoadParty(FileBuffer& fb)
{
    auto characters = LoadCharacters(fb);
    auto activeCharacters = LoadActiveCharacters(fb);
    auto gold = LoadGold(fb);
    auto keys = LoadCharacterInventory(fb, SaveOffsets::sPartyKeyInventoryOffset);
    auto party = Party{
        gold,
        std::move(keys),
        characters,
        activeCharacters};
    return party;
}

Royals LoadGold(FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sGoldOffset);
    return Royals{fb.GetUint32LE()};
}

std::vector<CharIndex> LoadActiveCharacters(FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sActiveCharactersOffset);
    const auto activeCharacters = fb.GetUint8();

    auto active = std::vector<CharIndex>{};
    for (unsigned i = 0; i < activeCharacters; i++)
    {
        const auto c = fb.GetUint8();
        active.emplace_back(c);
    }

    return active;
}

}
