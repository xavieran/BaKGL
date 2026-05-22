#pragma once

#include "bak/types.hpp"
#include "bak/party.hpp"

namespace BAK {

class FileBuffer;

Party LoadParty(FileBuffer&);
Royals LoadGold(FileBuffer&);
std::vector<CharIndex> LoadActiveCharacters(FileBuffer&);

}
