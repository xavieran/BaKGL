#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK::State {

bool ReadTempleSeen(FileBuffer&, unsigned temple);
void SetTempleSeen(FileBuffer&, unsigned temple);

}
