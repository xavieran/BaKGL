#include "bak/save/party.hpp"
#include "bak/save/character.hpp"
#include "bak/save/containers.hpp"

#include "bak/constants.hpp"
#include "bak/save/saveOffsets.hpp"
#include "bak/fileBufferFactory.hpp"

#include "com/logger.hpp"

namespace BAK {

Party LoadParty(FileBuffer& fb)
{
    auto inventories = LoadCharacterInventories(fb);
    auto characters = LoadCharacters(fb, inventories);
    auto activeCharacters = LoadActiveCharacters(fb);
    auto gold = LoadGold(fb);
    auto keys = LoadCharacterInventory(fb, SaveOffsets::sPartyKeyInventoryOffset);
    auto party = Party{
        gold,
        std::move(keys),
        std::move(inventories),
        std::move(characters),
        std::move(activeCharacters)};
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

void Save(const Party& party, FileBuffer& fb)
{
    fb.Seek(SaveOffsets::sGoldOffset);
    fb.PutUint32LE(party.GetGold().mValue);

    for (const auto& character : party.mCharacters)
    {
        Save(character, fb);
    }

    fb.Seek(BAK::SaveOffsets::sPartyKeyInventoryOffset);
    fb.PutUint8(party.GetKeys().GetInventory().GetNumberItems());
    fb.PutUint16LE(party.GetKeys().GetInventory().GetCapacity());
    Save(party.GetKeys().GetInventory(), fb);

    fb.Seek(SaveOffsets::sActiveCharactersOffset);
    fb.PutUint8(party.mActiveCharacters.size());
    for (const auto charIndex : party.mActiveCharacters)
        fb.PutUint8(charIndex.mValue);
}

}
