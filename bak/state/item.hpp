#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK::State {

bool ReadItemHasBeenUsed(FileBuffer&, unsigned character, unsigned itemIndex);
void SetItemHasBeenUsed(FileBuffer&, unsigned character, unsigned itemIndex);

}
