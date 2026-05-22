#pragma once

#include "bak/types.hpp"
#include "bak/character.hpp"
#include "bak/condition.hpp"
#include "bak/inventory.hpp"
#include "bak/skills.hpp"

#include <vector>

namespace BAK {

class FileBuffer;

std::vector<Character> LoadCharacters(FileBuffer&);
Inventory LoadCharacterInventory(FileBuffer&, unsigned offset);
Conditions LoadConditions(FileBuffer&, unsigned character);
std::vector<SkillAffector> GetCharacterSkillAffectors(FileBuffer&, CharIndex);

}
