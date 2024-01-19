#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK::State {

bool ReadSkillSelected(FileBuffer&, unsigned character, unsigned skill);
bool ReadSkillUnseenImprovement(FileBuffer&, unsigned character, unsigned skill);
void ClearUnseenImprovements(FileBuffer&, unsigned character);

std::uint8_t ReadSelectedSkillPool(FileBuffer&, unsigned character);
void SetSelectedSkillPool(FileBuffer&, unsigned character, std::uint8_t value);

}
