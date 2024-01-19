#pragma once

#include "bak/file/fileBuffer.hpp"

namespace BAK::State {

void SetLockHasBeenSeen(FileBuffer&, unsigned lockIndex);
bool CheckLockHasBeenSeen(FileBuffer&, unsigned lockIndex);

}

